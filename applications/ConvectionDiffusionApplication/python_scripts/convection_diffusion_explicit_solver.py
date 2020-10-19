# Importing the Kratos Library
import KratosMultiphysics

# Import applications
import KratosMultiphysics.ConvectionDiffusionApplication as ConvectionDiffusionApplication

# Import base class file
from KratosMultiphysics.ConvectionDiffusionApplication import (
    convection_diffusion_base_solver,
)


def CreateSolver(model, custom_settings):
    return ConvectionDiffusionExplicitSolver(model, custom_settings)


class ConvectionDiffusionExplicitSolver(
    convection_diffusion_base_solver.ConvectionDiffusionBaseSolver
):
    """
    The explicit class for convection-diffusion solvers.
    See convection_diffusion_base_solver.py for more information.
    """

    def __init__(self, model, custom_settings):
        # Construct the base solver and validate the remaining settings in the base class
        super().__init__(model, custom_settings)

        # Overwrite the base solver minimum buffer size
        self.min_buffer_size = 2

        element_name = self.settings["element_replace_settings"][
            "element_name"
        ].GetString()
        if self.settings["use_orthogonal_subscales"].GetBool() is True:
            if element_name in (
                "QSConvectionDiffusionExplicit",
                "DConvectionDiffusionExplicit",
            ):
                self.main_model_part.ProcessInfo.SetValue(
                    KratosMultiphysics.OSS_SWITCH, 1
                )
            else:
                err_msg = self.__class__.__name__
                err_msg += (
                    "The selected element",
                    element_name,
                    "does not support OSS projection. Select QSConvectionDiffusionExplicit or DConvectionDiffusionExplicit instead.",
                )
                raise Exception(err_msg)
        else:
            if element_name in (
                "QSConvectionDiffusionExplicit",
                "DConvectionDiffusionExplicit",
            ):
                self.main_model_part.ProcessInfo.SetValue(
                    KratosMultiphysics.OSS_SWITCH, 0
                )
            else:
                err_msg = self.__class__.__name__
                err_msg += (
                    "The selected element",
                    element_name,
                    "does not support OSS projection. Select QSConvectionDiffusionExplicit or DConvectionDiffusionExplicit instead.",
                )
                raise Exception(err_msg)

        KratosMultiphysics.Logger.PrintInfo(
            self.__class__.__name__, "Construction finished"
        )

    @classmethod
    def GetDefaultParameters(cls):
        default_settings = KratosMultiphysics.Parameters(
        """
        {
            "use_orthogonal_subscales" : false,
            "explicit_parameters" : {
                "dynamic_tau": 1.0
            }
        }
        """
        )

        default_settings.AddMissingParameters(super().GetDefaultParameters())
        return default_settings

    #### Private functions ####

    def _create_builder_and_solver(self):
        builder_and_solver = KratosMultiphysics.ExplicitBuilder()
        return builder_and_solver

    def _create_convection_diffusion_solution_strategy(self):
        convection_diffusion_solution_strategy = self._create_runge_kutta_4_strategy()
        return convection_diffusion_solution_strategy

    def _create_solution_scheme(self):
        self.GetComputingModelPart().ProcessInfo[
            KratosMultiphysics.DYNAMIC_TAU
        ] = self.settings["explicit_parameters"]["dynamic_tau"].GetDouble()

    def _create_runge_kutta_4_strategy(self):
        computing_model_part = self.GetComputingModelPart()
        convection_diffusion_scheme = (
            self.get_solution_scheme()
        )  # to call _create_solution_scheme method of the explicit solver
        explicit_builder_and_solver = self.get_builder_and_solver()
        rebuild_level = 0
        return ConvectionDiffusionApplication.ExplicitSolvingStrategyRungeKutta4ConvectionDiffusion(
            computing_model_part,
            explicit_builder_and_solver,
            self.settings["move_mesh_flag"].GetBool(),
            rebuild_level,
        )
