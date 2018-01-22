#pragma once
#define _SCL_SECURE_NO_WARNINGS

#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <memory>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkCommand.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <array>
#include <vtkProperty.h>
#include <vtkAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkCaptionActor2D.h>
#include <vtkPropCollection.h>
#include <vector>
#include <string>
#include <itkImage.h>
#include <vtkImageImport.h>
#include <itkCommand.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <fstream>
#include <itkMacro.h>
#include <itkOrientImageFilter.h>
#include <vtkOpenGLRenderer.h>
#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageSlabReslice.h>
#include "boost/date_time/posix_time/posix_time.hpp" //include all types plus i/o
#include <boost/lexical_cast.hpp>
#include <vtkXMLImageDataWriter.h>
#include <vtkMatrix4x4.h>
#include <assert.h>
#include <vtkImageActor.h>
#include <vtkImageProperty.h>
#include <vtkImageMapper3d.h>
#include <vtkObjectFactory.h>
#include <vtkLightsPass.h>
#include <vtkDefaultPass.h>
#include <vtkRenderPassCollection.h>
#include <vtkSequencePass.h>
#include <vtkCameraPass.h>
#include <vtkRenderState.h>
#include <SDL2\SDL_ttf.h>
#undef main
#include <SDL2/SDL.h>
#undef main
#include <vtkGeneralTransform.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkPolygon.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkHomogeneousTransform.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <itkRigid3DTransform.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>


static inline std::array<double, 3> operator+(const std::array<double, 3>& a, const std::array<double, 3>& b) {
	std::array<double, 3> c = { {a[0] + b[0], a[1] + b[1], a[2] + b[2]} };
	return c;
}

static inline std::array<double, 3> operator-(const std::array<double, 3>& a, const std::array<double, 3>& b) {
	std::array<double, 3> c = { { a[0] - b[0], a[1] - b[1], a[2] - b[2] } };
	return c;
}


static inline std::array<double, 3> operator*(const std::array<double, 3>& a, const double& b) {
	std::array<double, 3> c = { {a[0] * b, a[1] * b, a[2] * b} };
	return c;
}

static inline ostream& operator<<(ostream& os, const std::array<double,3>& data){
	os << "[" << data[0] << ", " << data[1] << ", " << data[2] << "]";
	return os;
}