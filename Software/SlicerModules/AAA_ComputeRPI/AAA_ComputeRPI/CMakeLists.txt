
#-----------------------------------------------------------------------------
set(MODULE_NAME AAA_ComputeRPI)

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#

#-----------------------------------------------------------------------------
set(MODULE_INCLUDE_DIRECTORIES
  ${vtkITK_INCLUDE_DIRS}
  )

set(MODULE_SRCS
  )

set(MODULE_TARGET_LIBRARIES
  ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  ADDITIONAL_SRCS ${MODULE_SRCS}
  )

#-----------------------------------------------------------------------------

