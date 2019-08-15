SOURCES += ./ops/Domain.cpp \
    ops/DirectIntegrationAnalysis.cpp \
    ops/Newmark.cpp \
    ops/TransientAnalysis.cpp \
    ops/TransientIntegrator.cpp \
    ops/PlainHandler.cpp \
    ops/PathSeries.cpp \
    ops/UniformExcitation.cpp \
    ops/GroundMotion.cpp \
    ops/TrapezoidalTimeSeriesIntegrator.cpp \
    ops/EarthquakePattern.cpp \
    ops/TimeSeriesIntegrator.cpp

SOURCES += ./ops/Analysis.cpp
SOURCES += ./ops/AnalysisModel.cpp
SOURCES += ./ops/ArrayOfTaggedObjects.cpp
SOURCES += ./ops/ArrayOfTaggedObjectsIter.cpp
SOURCES += ./ops/BandGenLinLapackSolver.cpp
SOURCES += ./ops/BandGenLinSOE.cpp
SOURCES += ./ops/BandGenLinSolver.cpp
SOURCES += ./ops/BeamIntegration.cpp
SOURCES += ./ops/Channel.cpp
SOURCES += ./ops/CompositeResponse.cpp
SOURCES += ./ops/ConstraintHandler.cpp
SOURCES += ./ops/ConvergenceTest.cpp
SOURCES += ./ops/CorotCrdTransf2d.cpp
SOURCES += ./ops/CorotTruss.cpp
SOURCES += ./ops/CrdTransf.cpp
SOURCES += ./ops/CTestNormDispIncr.cpp
SOURCES += ./ops/CTestEnergyIncr.cpp
SOURCES += ./ops/DispBeamColumn2d.cpp
SOURCES += ./ops/DisplacementControl.cpp
SOURCES += ./ops/DOF_Group.cpp
SOURCES += ./ops/DOF_GrpIter.cpp
SOURCES += ./ops/DOF_Numberer.cpp
SOURCES += ./ops/DomainComponent.cpp
SOURCES += ./ops/DummyStream.cpp
SOURCES += ./ops/DummyElementAPI.cpp
SOURCES += ./ops/EigenSOE.cpp
SOURCES += ./ops/EigenSolver.cpp
SOURCES += ./ops/ElasticSection2d.cpp
SOURCES += ./ops/ElasticBeam2d.cpp
SOURCES += ./ops/Element.cpp
SOURCES += ./ops/ElementalLoadIter.cpp
SOURCES += ./ops/ElementResponse.cpp
SOURCES += ./ops/EquiSolnAlgo.cpp
SOURCES += ./ops/FatigueMaterial.cpp
SOURCES += ./ops/FE_EleIter.cpp
SOURCES += ./ops/FE_Element.cpp
SOURCES += ./ops/Fiber.cpp
SOURCES += ./ops/FiberResponse.cpp
SOURCES += ./ops/FiberSection2d.cpp
SOURCES += ./ops/ForceBeamColumn2d.cpp
SOURCES += ./ops/Graph.cpp
SOURCES += ./ops/GraphNumberer.cpp
SOURCES += ./ops/ID.cpp
SOURCES += ./ops/IncrementalIntegrator.cpp
SOURCES += ./ops/Information.cpp
SOURCES += ./ops/Integrator.cpp
SOURCES += ./ops/KrylovNewton.cpp
SOURCES += ./ops/LegendreBeamIntegration.cpp
SOURCES += ./ops/LinearCrdTransf3d.cpp
SOURCES += ./ops/LinearSeries.cpp
SOURCES += ./ops/LinearSOE.cpp
SOURCES += ./ops/LinearSOESolver.cpp
SOURCES += ./ops/Load.cpp
SOURCES += ./ops/LoadControl.cpp
SOURCES += ./ops/LoadPattern.cpp
SOURCES += ./ops/LoadPatternIter.cpp
SOURCES += ./ops/LobattoBeamIntegration.cpp
SOURCES += ./ops/MapOfTaggedObjects.cpp
SOURCES += ./ops/MapOfTaggedObjectsIter.cpp
SOURCES += ./ops/Material.cpp
SOURCES += ./ops/MaterialResponse.cpp
SOURCES += ./ops/Matrix.cpp
SOURCES += ./ops/MatrixUtil.cpp
SOURCES += ./ops/MovableObject.cpp
SOURCES += ./ops/MP_Constraint.cpp
SOURCES += ./ops/NewtonLineSearch.cpp
SOURCES += ./ops/NewtonRaphson.cpp
SOURCES += ./ops/NodalLoad.cpp
SOURCES += ./ops/NodalLoadIter.cpp
SOURCES += ./ops/Node.cpp
SOURCES += ./ops/OPS_Stream.cpp
SOURCES += ./ops/PenaltyConstraintHandler.cpp
SOURCES += ./ops/PathTimeSeries.cpp
SOURCES += ./ops/PenaltyMP_FE.cpp
SOURCES += ./ops/PenaltySP_FE.cpp
SOURCES += ./ops/PlainNumberer.cpp
SOURCES += ./ops/ProfileSPDLinDirectSolver.cpp
SOURCES += ./ops/ProfileSPDLinSOE.cpp
SOURCES += ./ops/ProfileSPDLinSolver.cpp
SOURCES += ./ops/Response.cpp
SOURCES += ./ops/RCM.cpp
SOURCES += ./ops/SectionForceDeformation.cpp
SOURCES += ./ops/SectionIntegration.cpp
SOURCES += ./ops/SingleDomSP_Iter.cpp
SOURCES += ./ops/SingleDomMP_Iter.cpp
SOURCES += ./ops/SingleDomNodIter.cpp
SOURCES += ./ops/SingleDomEleIter.cpp
SOURCES += ./ops/SingleDomParamIter.cpp
SOURCES += ./ops/SingleDomAllSP_Iter.cpp
SOURCES += ./ops/SingleDomPC_Iter.cpp
SOURCES += ./ops/SolutionAlgorithm.cpp
SOURCES += ./ops/SP_Constraint.cpp
SOURCES += ./ops/StandardStream.cpp
SOURCES += ./ops/StaticAnalysis.cpp
SOURCES += ./ops/StaticIntegrator.cpp
SOURCES += ./ops/Steel01.cpp
SOURCES += ./ops/Steel02.cpp
SOURCES += ./ops/Steel4.cpp
SOURCES += ./ops/Subdomain.cpp
SOURCES += ./ops/SubdomainNodIter.cpp
SOURCES  += ./ops/SymBandEigenSOE.cpp
SOURCES  += ./ops/SymBandEigenSolver.cpp
SOURCES += ./ops/TaggedObject.cpp
SOURCES += ./ops/TimeSeries.cpp
SOURCES += ./ops/TransformationConstraintHandler.cpp
SOURCES += ./ops/TransformationDOF_Group.cpp
SOURCES += ./ops/TransformationFE.cpp
SOURCES += ./ops/UniaxialFiber2d.cpp
SOURCES += ./ops/UniaxialFiber3d.cpp
SOURCES += ./ops/UniaxialMaterial.cpp
SOURCES += ./ops/Vector.cpp
SOURCES += ./ops/Vertex.cpp
SOURCES += ./ops/VertexIter.cpp
SOURCES += ./ops/ZeroLength.cpp
SOURCES += ./ops/PlaneStressRebarMaterial.cpp
SOURCES += ./ops/NDMaterial.cpp

HEADERS += \
        ops/Analysis.h \
        ops/AnalysisModel.h \
        ops/ArrayOfTaggedObjects.h \
        ops/ArrayOfTaggedObjectsIter.h \
        ops/bool.h \
        ops/BandGenLinLapackSolver.h \
        ops/BandGenLinSOE.h \
        ops/BandGenLinSolver.h \
        ops/BeamFiberMaterial.h \
        ops/BeamFiberMaterial2d.h \
        ops/BeamIntegration.h \
        ops/BinaryFileStream.h \
        ops/Channel.h \
        ops/ColorMap.h \
        ops/CompositeResponse.h \
        ops/ConstraintHandler.h \
        ops/ConvergenceTest.h \
        ops/CorotCrdTransf2d.h \
        ops/CorotTruss.h \
        ops/CrdTransf.h \
        ops/CTestEnergyIncr.h \
        ops/CTestNormDispIncr.h \
        ops/DOF_Group.h \
        ops/DOF_GrpIter.h \
        ops/DOF_Numberer.h \
        ops/DataFileStream.h \
        ops/DispBeamColumn3d.h \
        ops/DispBeamColumn2d.h \
        ops/DispBeamColumn3dWithSensitivity.h \
        ops/DisplacementControl.h \
        ops/Domain.h \
        ops/DomainComponent.h \
        ops/DomainDecompositionAnalysis.h \
        ops/DummyStream.h \
        ops/EigenSOE.h \
        ops/EigenSolver.h \
        ops/ElasticBeam2d.h \
        ops/ElasticSection2d.h \
        ops/Element.h \
        ops/ElementIter.h \
        ops/ElementResponse.h \
        ops/ElementalLoad.h \
        ops/ElementalLoadIter.h \
        ops/EquiSolnAlgo.h \
        ops/FatigueMaterial.h \
        ops/FEM_ObjectBroker.h \
        ops/FE_Datastore.h \
        ops/FE_EleIter.h \
        ops/FE_Element.h \
        ops/Fiber.h \
        ops/FiberResponse.h \
        ops/FiberSection2D.h \
        ops/File.h \
        ops/FileIter.h \
        ops/ForceBeamColumn2d.h \
        ops/FrictionModel.h \
        ops/FrictionResponse.h \
        #ops/G3Globals.h \
        ops/Graph.h \
        ops/GraphNumberer.h \
        ops/ID.h \
        ops/IncrementalIntegrator.h \
        ops/Information.h \
        ops/Integrator.h \
        ops/KrylovNewton.h \
        ops/LegendreBeamIntegration.h \
        ops/LinearCrdTransf3d.h \
        ops/LinearSOE.h \
        ops/LinearSOESolver.h \
        ops/LinearSeries.h \
        ops/Load.h \
        ops/LoadControl.h \
        ops/LoadPattern.h \
        ops/LoadPatternIter.h \
        ops/LobattoBeamIntegration.h \
        ops/MP_Constraint.h \
        ops/MP_ConstraintIter.h \
        ops/MapOfTaggedObjects.h \
        ops/MapOfTaggedObjectsIter.h \
        ops/Material.h \
        ops/MaterialResponse.h \
        ops/Matrix.h \
        ops/MatrixUtil.h \
        ops/MeshRegion.h \
        ops/Message.h \
        ops/MovableObject.h \
        ops/NDMaterial.h \
        ops/NewtonLineSearch.h \
        ops/NewtonRaphson.h \
        ops/NodalLoad.h \
        ops/NodalLoadIter.h \
        ops/Node.h \
        ops/NodeIter.h \
        ops/OPS_Globals.h \
        ops/OPS_Stream.h \
        ops/ObjectBroker.h \
        ops/PathTimeSeries.h \
        ops/Parameter.h \
        ops/ParameterIter.h \
        ops/PenaltyConstraintHandler.h \
        ops/PenaltyMP_FE.h \
        ops/PenaltySP_FE.h \
        ops/PlainMap.h \
        ops/PlainNumberer.h \
        ops/PlaneStrainMaterial.h \
        ops/PlaneStressMaterial.h \
        ops/PlateFiberMaterial.h \
        ops/Pressure_Constraint.h \
        ops/Pressure_ConstraintIter.h \
        ops/PySimple1.h \
        ops/QzSimple1.h \
        ops/RCM.h \
        ops/Recorder.h \
        ops/Renderer.h \
        ops/Response.h \
        ops/SP_Constraint.h \
        ops/SP_ConstraintIter.h \
        ops/SectionForceDeformation.h \
        ops/FiberSection2d.h \
        ops/SimulationInformation.h \
        ops/SingleDomAllSP_Iter.h \
        ops/SingleDomEleIter.h \
        ops/SingleDomLC_Iter.h \
        ops/SingleDomMP_Iter.h \
        ops/SingleDomNodIter.h \
        ops/SingleDomPC_Iter.h \
        ops/SingleDomParamIter.h \
        ops/SingleDomSP_Iter.h \
        ops/SolutionAlgorithm.h \
        ops/StandardStream.h \
        ops/StaticAnalysis.h \
        ops/StaticIntegrator.h \
        ops/Steel01.h \
        ops/Steel02.h \
        ops/Steel4.h \
        ops/StringContainer.h \
        ops/Subdomain.h \
        ops/SubdomainNodIter.h \
        ops/TaggedObject.h \
        ops/TaggedObjectIter.h \
        ops/TaggedObjectStorage.h \
        ops/TimeSeries.h \
        ops/TransformationConstraintHandler.h \
        ops/TransformationDOF_Group.h \
        ops/TransformationFE.h \
        ops/TransientIntegrator.h \
        ops/UniaxialFiber2d.h \
        ops/UniaxialFiber3d.h \
        ops/UniaxialMaterial.h \
        ops/Vector.h \
        ops/Vertex.h \
        ops/VertexIter.h \
        ops/ZeroLength.h \
        ops/classTags.h \
        ops/elementAPI.h \
        ops/OPS_Globals.h \
        ops/PlaneStressRebarMaterial.h

#SOURCES += ./ops/BandGenLinSOE.cpp
#SOURCES += ./ops/BandGenLinSolver.cpp
#SOURCES += ./ops/BandGenLinLapackSolver.cpp
#DEFINES += _FORTRAN_LIBS

DEFINES += _bool_h
INCLUDEPATH += "./ops"



unix {

macx {

DEFINES += "_MacOSX"

#LIBS += /Users/fmk/lib/libLapack.a
#LIBS += /Users/fmk/lib/libBlas.a
#LIBS += /usr/local/llvm-gcc/lib/libgfortran.a
LIBS += -llapack
LIBS += -lblas


INCLUDEPATH += .include "./ops"
#INCLUDEPATH += "$(HOME)/OpenSees/DEVELOPER/core"

} else {


DEFINES += "_LINUX"


INCLUDEPATH += "/apps/rappture/dev/include"
INCLUDEPATH += ./include "./ops"

LIBS += -llapack
LIBS += -lblas
LIBS += -lgfortran

SRC += ./ops/Domain.cpp

}
}

win32 {


INCLUDEPATH += C:\Progra~1\Tcl\include
INCLUDEPATH += $$(HOME)\OpenSees\DEVELOPER\core
INCLUDEPATH += "./ops"
#LIBS += c:\Users\SimCenter\OpenSees\Win32\lib\release\lapack.lib
#LIBS += c:\Users\SimCenter\OpenSees\Win32\lib\release\blas.lib
#LIBS += c:\Users\SimCenter\OpenSees\Win32\lib\libifcoremt.lib
#LIBS += c:\Users\SimCenter\OpenSees\Win32\lib\libirc.lib
#LIBS += c:\Users\SimCenter\OpenSees\Win32\lib\ifconsol.lib
#LIBS += c:\Users\SimCenter\OpenSees\Win32\lib\libifport.lib
#LIBS += c:\Users\SimCenter\OpenSees\Win32\lib\libmmt.lib
LIBS += c:\Users\Simpson\OpenSees\x64\lapack.lib
LIBS += c:\Users\Simpson\OpenSees\x64\blas.lib
LIBS += c:\Users\Simpson\OpenSees\x64\libifcoremt.lib
LIBS += c:\Users\Simpson\OpenSees\x64\libirc.lib
LIBS += c:\Users\Simpson\OpenSees\x64\ifconsol.lib
LIBS += c:\Users\Simpson\OpenSees\x64\libifport.lib
LIBS += c:\Users\Simpson\OpenSees\x64\libmmt.lib
LIBS += c:\Users\Simpson\OpenSees\x64\svml_dispmt.lib
# NOTE THAT THE OpenSees libs have to be created with /MD as opposed to /MT as runtime library
# this is specified in C++ -> Code Generation -> RunTime Library
# this is because Qt must be built with this option as they have memory issues if built /MT

#QMAKE_CXXFLAGS += /GS /W3 /Gy   /Zi /Gm- /O2 /Ob1 /fp:precise /errorReport:prompt /GF /WX- /Zc:forScope /Gd /EHsc /MD

CONFIG += static
#QMAKE_LFLAGS_DEBUG += _DLL
QMAKE_LFLAGS_DEBUG += /FORCE:MULTIPLE
QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:"libc.lib"
QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:"libcpmt.lib"

#QMAKE_LFLAGS_RELEASE += _DLL
QMAKE_LFLAGS_RELEASE += /FORCE:MULTIPLE
QMAKE_LFLAGS_RELEASE += /NODEFAULTLIB:"libc.lib"
QMAKE_LFLAGS_RELEASE += /NODEFAULTLIB:"libcpmt.lib"
#LIBPATH += "C:\Program Files (x86)\Intel\Composer XE\compiler\lib\intel64"

DEFINES += _WIN32

} else {

}

               
