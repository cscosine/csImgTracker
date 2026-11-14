#!/usr/bin/env python3
import sys
from collections.abc import Sequence
from pathlib import Path

from csorchestrator.application.cli.cli import orchestrator_main_with_default_run
from csorchestrator.application.factory.factory import (
    OptionalOrchestratorWithReport,
    create_orchestrator_factory_all_supported_cases,
)
from csorchestrator.application.recipes.checkout_build import (
    build_repos,
    checkout_repos,
    create_and_upload_artifacts,
    create_and_upload_deployments,
)
from csorchestrator.application.recipes.create_orchestrator import (
    create_default_orchestrator,
)
from csorchestrator.application.recipes.cscmake_release import create_cscmake_release_archive
from csorchestrator.application.recipes.manifest_github import install_ubuntu_apt_packages
from csorchestrator.domain.context.context_os_architecture import OS
from csorchestrator.foundation.core.report import Report
from csorchestrator.frontend.cscmake_presets.supported_variants import (
    BuildConfig,
)
from csorchestrator.frontend.local_execution.step_utils import StepExecuteOnlyOn, StepSkipExecutionOnLocal
from csorchestrator.frontend.step.step_custom_command import StepInstallAptPackages
from csorchestrator.frontend.step.step_deployments import StepRunDeployment
from csorchestrator.frontend.step.step_get_precompiled_lib_github import (
    identity_mapping_function,
)

from csImgTracker_config import install_csimgtracker_build_dependencies


def create_orchestrator() -> OptionalOrchestratorWithReport:
    report = Report()

    base_target_dir = Path("workspace")
    base_install_dir = base_target_dir / Path("install")
    base_libs_dir = base_target_dir / Path("libs")
    base_build_dir = base_target_dir / Path("build")
    deploy_archives_dir = base_target_dir / Path("deploy-archives")
    build_config = BuildConfig.DEBUG_RELEASE_RELWITHDEBINFO_PARANOID
    cscmake_version = "1.0.0-rc4"

    o = create_orchestrator_factory_all_supported_cases(
        name="csImgTracker",
        version="0.1.0",
        execution_matrix_name="orchestrator-matrix",
    )

    o = create_default_orchestrator(
        name="csImgTracker",
        version="0.1.0",
        base_install_dir=base_install_dir,
    )

    # ----------------------------------------------------------------
    checkout_repos(
        orchestrator=o,
        base_target_dir=base_target_dir,
        repo_access_token="${{ secrets.ACTIONS_ORG_ACCESS }}",
        release_archive_list=[create_cscmake_release_archive(cscmake_version)],
    )

    report.append_report(
        install_csimgtracker_build_dependencies(
            orchestrator=o,
            base_libs_dir=base_libs_dir,
            dependencies_config_path=Path("dependencies"),
            dev=True,
            download_dependencies_manifest_and_bundle=True,
            mapping_function=identity_mapping_function,
            phase_name_suffix="csImgTracker dependencies",
        )
    )

    # the libraries of the system the Qt xcb plugin needs: the deployment (cs_deployment) resolves the plugins, and
    # fails when one is missing on the build machine
    install_ubuntu_apt_packages(
        orchestrator=o,
        packages=[
            "libxcb-cursor0",
            "libxcb-icccm4",
            "libxcb-image0",
            "libxcb-keysyms1",
            "libxcb-render-util0",
            "libxcb-shape0",
            "libxcb-xkb1",
            "libxkbcommon-x11-0",
        ],
        phase_name_suffix="Qt xcb plugin",
    )

    # ----------------------------------------------------------------
    build_repos(
        orchestrator=o,
        base_target_dir=base_target_dir,
        build_self=build_config,
    )

    create_and_upload_artifacts(
        orchestrator=o,
        base_install_dir=base_install_dir,
        repos_auto_search_list=["csImgTracker"],
    )

    # ----------------------------------------------------------------
    # deployments (cs_deployment in csImgTracker/CMakeLists.txt, made by the install): archived, uploaded, attached
    # to the release; then run, extracted from their archive, with a clean environment
    create_and_upload_deployments(
        orchestrator=o,
        base_build_dir=base_build_dir,
        build_config=build_config,
        output_dir=deploy_archives_dir,
    )

    p = o.create_phase("Run Deployments")
    # Linux CI: a virtual X server for the smoke run (OpenGL through Mesa); not installed on a developer machine
    p.add_step(
        StepInstallAptPackages(
            name="install xvfb",
            description="virtual X server for the smoke run of the deployment",
            packages=["xvfb", "xauth"],
        )
        .add_extra(StepExecuteOnlyOn(os=OS.LINUX))
        .add_extra(StepSkipExecutionOnLocal())
    )
    p.add_step(
        StepRunDeployment(
            name="Smoke run of the deployment app",
            description="start csImgTracker from its deployment: its OpenGL view gets a context, exit code 0",
            deployment="app",
            # Linux: xvfb-run when there (CI), else the display of the session (a developer machine); Windows: the
            # desktop of the session, the software OpenGL of Qt (QT_OPENGL, no GPU on the CI runners)
            command={
                OS.LINUX: [
                    "sh",
                    "-c",
                    'if command -v xvfb-run >/dev/null; then exec xvfb-run -a "$0" --smoke; else exec "$0" --smoke; fi',
                    "{folder}/bin/csImgTracker",
                ],
                OS.WINDOWS: ["{folder}/bin/csImgTracker.exe", "--smoke"],
            },
            # QT_FORCE_STDERR_LOGGING: the messages of Qt in the log (a GUI application on Windows: not on the console)
            environment={"QT_OPENGL": "software", "QT_FORCE_STDERR_LOGGING": "1"},
            base_build_dir=base_build_dir,
            build_config=build_config,
            archives_dir=deploy_archives_dir,
            install_system_requirements=True,
        )
    )

    # single return point
    if report.has_errors():
        return OptionalOrchestratorWithReport.create_report(report)
    return OptionalOrchestratorWithReport.create_result_and_report(o, report)


def main(argv: Sequence[str] | None = None) -> int:
    script_path = str(Path(__file__).resolve())
    return orchestrator_main_with_default_run(script_path, argv)


if __name__ == "__main__":
    sys.exit(main())
