#include "stdafx.h"
#include "loadVolume.h"
#include "myResliceCube.h"
class ObserveLoadProgressCommand : public itk::Command
{
public:
	itkNewMacro(ObserveLoadProgressCommand);
	void Execute(itk::Object * caller, const itk::EventObject & event)
	{
		Execute((const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * caller, const itk::EventObject & event)
	{
		if (!itk::ProgressEvent().CheckEvent(&event))
		{
			return;
		}
		const itk::ProcessObject * processObject =
			dynamic_cast< const itk::ProcessObject * >(caller);
		if (!processObject)
		{
			return;
		}
		std::cout << processObject->GetProgress() << std::endl;
	}
};
vtkSmartPointer<vtkVolume> CreateAVolume(vtkSmartPointer<vtkImageImport>src);
vtkSmartPointer<vtkActor> CreateBoundingBox(vtkVolume* v);
vtkSmartPointer<vtkActor> CreateResliceCube(double *initialPosition);

class myCubeCallback : public vtkCommand {
private:
	myCubeCallback() {
		cubeActor = nullptr;
		boundsDoVolume = nullptr;
		imageSource = nullptr;
	}
public:
	vtkImageImport* imageSource;
	vtkActor* cubeActor;
	double* boundsDoVolume;
	static myCubeCallback *New() {
		return new myCubeCallback();
	}

	bool isInsideBounds(std::array<double, 3> p, double bounds[6]) {
		if ((bounds[0] < p[0] && p[0] < bounds[1]) &&
			(bounds[2] < p[1] && p[1] < bounds[3]) &&
			(bounds[4] < p[2] && p[2] < bounds[5])) {
			return true;
		}
		else {
			return false;
		}
	}
	std::pair<std::array<double, 3>, double> MarchVectorUntilBorder(std::array<double, 3> c, std::array<double, 3> v, double* bounds) {
		double gamma = 0.0;
		std::array<double, 3> p;
		while (true) {
			p = c + v * gamma;
			if (isInsideBounds(p, bounds)) {
				//incrementa e continua
				gamma = gamma + 0.1;
			}
			else {
				//para
				break;
			}
		}
		return std::make_pair(p, gamma);
	}
	//Isso aqui é chamado toda vez que o renderer do cubo de reslice renderiza
	void Execute(vtkObject * caller, unsigned long event, void* calldata) {
		////qual é o centro?
		const std::array<double, 3> center = { {cubeActor->GetCenter()[0],cubeActor->GetCenter()[1], cubeActor->GetCenter()[2], } };
		////qual é o vetor?
		const std::array<double, 3> u = { { cubeActor->GetMatrix()->Element[0][0], cubeActor->GetMatrix()->Element[1][0],cubeActor->GetMatrix()->Element[2][0],} };
		const auto uMarch = MarchVectorUntilBorder(center, u, boundsDoVolume);
		const std::array<double, 3> uNeg = { { -cubeActor->GetMatrix()->Element[0][0], -cubeActor->GetMatrix()->Element[1][0], -cubeActor->GetMatrix()->Element[2][0], } };
		const auto uNegMarch = MarchVectorUntilBorder(center, uNeg, boundsDoVolume);

		const std::array<double, 3> v = { { cubeActor->GetMatrix()->Element[0][1], cubeActor->GetMatrix()->Element[1][1],cubeActor->GetMatrix()->Element[2][1], } };
		const auto vMarch = MarchVectorUntilBorder(center, v, boundsDoVolume);
		const std::array<double, 3> vNeg = { { -cubeActor->GetMatrix()->Element[0][1], -cubeActor->GetMatrix()->Element[1][1], -cubeActor->GetMatrix()->Element[2][1], } };
		const auto vNegMarch = MarchVectorUntilBorder(center, vNeg, boundsDoVolume);

		const std::array<double,3> xVector = { {abs(uMarch.first[0] - uNegMarch.first[0]),
			abs(uMarch.first[1] - uNegMarch.first[1]),
			abs(uMarch.first[2] - uNegMarch.first[2]),
			} };
		const std::array<double, 3> yVector = { { abs(vMarch.first[0] - vNegMarch.first[0]),
			abs(vMarch.first[1] - vNegMarch.first[1]),
			abs(vMarch.first[2] - vNegMarch.first[2]),
			} };
		const double normH = sqrt(xVector[0] * xVector[0] + xVector[1] * xVector[1] + xVector[2] * xVector[2]);
		const double normV = sqrt(yVector[0] * yVector[0] + yVector[1] * yVector[1] + yVector[2] * yVector[2]);
		/////Agora que eu tenho os tamanhos da horizontal e da vertical da imagem eu posso fazer o extent do output
		/////O spacing vai ser inicialmente (1,1,1)
		/////O center é onde está o center do cubo
		vtkSmartPointer<vtkImageSlabReslice> thickSlabReslice = vtkSmartPointer<vtkImageSlabReslice>::New();		
		thickSlabReslice->SetInputConnection(imageSource->GetOutputPort());
		thickSlabReslice->SetOutputSpacing(1, 1, 1);
		vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
		mat->DeepCopy(cubeActor->GetMatrix());
		thickSlabReslice->SetResliceAxes(mat);

		thickSlabReslice->SetResliceAxesOrigin(center[0], center[1], center[2]);
		thickSlabReslice->SetOutputDimensionality(2);
		thickSlabReslice->SetOutputExtent(0, normH*1.125, 0, normV*1.125, 0, 1);
		thickSlabReslice->Update();
		
		////grava no disco
		boost::posix_time::ptime current_date_microseconds = boost::posix_time::microsec_clock::local_time();
		long milliseconds = current_date_microseconds.time_of_day().total_milliseconds();
		std::string filename = "c:\\mprcubov2\\dump\\" + boost::lexical_cast<std::string>(milliseconds) + ".vti";
		vtkSmartPointer<vtkXMLImageDataWriter> debugsave = vtkSmartPointer<vtkXMLImageDataWriter>::New();
		debugsave->SetFileName(filename.c_str());
		debugsave->SetInputConnection(thickSlabReslice->GetOutputPort());
		debugsave->BreakOnError();
		debugsave->Write();
		/////Tendo os pontos onde raios saindo do centro encontram os limites do volume eu tenho informações suficientes para calcular extent do reslice
		////const std::array<double, 6> resliceOutputExtent = { {
		////		0, vtkMath::Norm()
		////	} };
		//std::cout << "--------" << std::endl;
		//std::cout << "horizontal da imagem = " << xVector<<" tamanho = "<<normH << std::endl;
		//std::cout << "vertical da imagem = " << yVector <<" tamanho = "<<normV << std::endl;
		//std::cout << "uMarch = " << uMarch.first <<" gamma = "<< uMarch.second<< std::endl;
		//std::cout << "uNegMarch = " << uNegMarch.first << " gamma = " << uNegMarch.second << std::endl;
		//std::cout << "vMarch = " << vMarch.first << " gamma = " << vMarch.second << std::endl;
		//std::cout << "vNegMarch = " << vNegMarch.first << " gamma = " << vNegMarch.second << std::endl;
	}
};

int main(int argc, char** argv) {
	///Carga da imagem
	ObserveLoadProgressCommand::Pointer prog = ObserveLoadProgressCommand::New();
	const std::string txtFile = "C:\\meus dicoms\\Marching Man"; //"C:\\meus dicoms\\Abd-Pel w-c  3.0  B30f";// "C:\\meus dicoms\\Marching Man"; /*"C:\\meus dicoms\\Abd-Pel w-c  3.0  B30f";*//*"C:\\meus dicoms\\abdomem-feet-first"*/;//"C:\\meus dicoms\\Abd-Pel w-c  3.0  B30f";//"C:\\meus dicoms\\Marching Man";//"C:\\meus dicoms\\Abd-Pel w-c  3.0  B30f";/*"C:\\meus dicoms\\Marching Man"*/; //"C:\\meus dicoms\\abdomem-feet-first";//"C:\\meus dicoms\\Marching Man"; //"C:\\meus dicoms\\Marching Man";//
	const std::vector<std::string> lst = GetList(txtFile);
	std::map<std::string, std::string> metadataDaImagem;
	itk::Image<short, 3>::Pointer imagemOriginal = LoadVolume(metadataDaImagem, lst, prog);
	///Reorienta a imagem
	itk::OrientImageFilter<itk::Image<short, 3>, itk::Image<short, 3>>::Pointer orienter = itk::OrientImageFilter<itk::Image<short, 3>, itk::Image<short, 3>>::New();
	orienter->AddObserver(itk::ProgressEvent(), prog);
	orienter->UseImageDirectionOn();
	orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP);
	orienter->SetInput(imagemOriginal);
	orienter->Update();
	imagemOriginal = orienter->GetOutput();
	//////fim da Carga da imagem - aqui a imagem já está carregada e orientada em uma orientação padrão.
	vtkSmartPointer<vtkImageImport> imagemImportadaPraVTK = CreateVTKImage(imagemOriginal);//importa a imagem da itk pra vtk.
	imagemImportadaPraVTK->Update();
	//Cria o volume3d pra testes
	vtkSmartPointer<vtkVolume> volumeActor = CreateAVolume(imagemImportadaPraVTK);
	//Cria uma caixa ao redor do volume
	vtkSmartPointer<vtkActor> actorBoundsVolume = CreateBoundingBox(volumeActor);

	//A tela do volume renderer
	vtkSmartPointer<vtkRenderer> rendererSistema = vtkSmartPointer<vtkRenderer>::New();
	rendererSistema->GetActiveCamera()->ParallelProjectionOn();
	rendererSistema->SetBackground(0.25, 0.125, 0.0625);
	vtkSmartPointer<vtkRenderWindow> renderWindowSistema = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowSistema->AddRenderer(rendererSistema);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractorSistema = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowSistema->SetInteractor(renderWindowInteractorSistema);
	renderWindowInteractorSistema->Initialize();
	renderWindowSistema->Render();
	rendererSistema->AddVolume(volumeActor);
	rendererSistema->AddActor(actorBoundsVolume);
	rendererSistema->ResetCamera();

	//A tela do cubo
	vtkSmartPointer<vtkOpenGLRenderer> rendererImagem = vtkSmartPointer<vtkOpenGLRenderer>::New();
	rendererImagem->SetLayer(0);
	rendererImagem->GetActiveCamera()->ParallelProjectionOn();
	vtkSmartPointer<vtkOpenGLRenderer> rendererCubo = vtkSmartPointer<vtkOpenGLRenderer>::New();
	rendererCubo->SetLayer(1);
	rendererCubo->GetActiveCamera()->ParallelProjectionOn();
	vtkSmartPointer<vtkWin32OpenGLRenderWindow> renderWindowCubeReslicer = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();
	renderWindowCubeReslicer->SetNumberOfLayers(2);
	renderWindowCubeReslicer->AddRenderer(rendererImagem);
	renderWindowCubeReslicer->AddRenderer(rendererCubo);
	renderWindowCubeReslicer->SetPosition(300, 0);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractorCubeReslicer = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkInteractorStyleTrackballActor> cubeReslicerStyle = vtkSmartPointer<vtkInteractorStyleTrackballActor>::New();
	renderWindowInteractorCubeReslicer->SetInteractorStyle(cubeReslicerStyle);
	renderWindowCubeReslicer->SetInteractor(renderWindowInteractorCubeReslicer);
	renderWindowInteractorCubeReslicer->Initialize();
	std::shared_ptr<myResliceCube> myCube = std::make_shared<myResliceCube>();
	myCube->SetRenderers(rendererCubo, rendererImagem);
	myCube->SetSource(imagemImportadaPraVTK);
	/*vtkSmartPointer<myCubeCallback> cubeCallback = vtkSmartPointer<myCubeCallback>::New();
	rendererCubeReslicer->AddObserver(vtkCommand::EndEvent, cubeCallback);
	cubeCallback->cubeActor = actorCuboReslice;
	cubeCallback->boundsDoVolume = volumeActor->GetBounds();
	cubeCallback->imageSource = imagemImportadaPraVTK;
*/
	//A tela dummy
	vtkSmartPointer<vtkRenderer> rendererDummy = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindowDummy = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowDummy->AddRenderer(rendererDummy);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractorDummy = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowDummy->SetInteractor(renderWindowInteractorDummy);
	renderWindowInteractorDummy->Initialize();
	renderWindowDummy->SetPosition(0, 400);
	renderWindowInteractorDummy->Start();

	return EXIT_SUCCESS;
}

vtkSmartPointer<vtkActor> CreateResliceCube(double *initialPosition) {
	vtkSmartPointer<vtkCubeSource> src = vtkSmartPointer<vtkCubeSource>::New();
	src->SetCenter(initialPosition);
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(src->GetOutputPort());
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetRepresentationToWireframe();
	actor->GetProperty()->SetColor(1, 0, 0);
	actor->GetProperty()->ShadingOff();
	return actor;
}

vtkSmartPointer<vtkActor> CreateBoundingBox(vtkVolume* v) {
	vtkSmartPointer<vtkCubeSource> src = vtkSmartPointer<vtkCubeSource>::New();
	src->SetCenter(v->GetCenter());
	src->SetXLength(v->GetMaxXBound() - v->GetMinXBound());
	src->SetYLength(v->GetMaxYBound() - v->GetMinYBound());
	src->SetZLength(v->GetMaxZBound() - v->GetMinZBound());
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(src->GetOutputPort());
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetRepresentationToWireframe();
	actor->GetProperty()->SetColor(1, 0, 0);
	return actor;
}

vtkSmartPointer<vtkVolume> CreateAVolume(vtkSmartPointer<vtkImageImport>src) {
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetInputConnection(src->GetOutputPort());
	vtkSmartPointer<vtkVolumeProperty> volumeProperties = vtkSmartPointer<vtkVolumeProperty>::New();
	vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
	ctf->AddRGBSegment(0, 0, 0, 0, 2500, 1, 1, 1);
	vtkSmartPointer<vtkPiecewiseFunction> sof = vtkSmartPointer<vtkPiecewiseFunction>::New();
	sof->AddSegment(0, 0, 2500, 0.25);
	volumeProperties->SetColor(ctf);
	volumeProperties->SetScalarOpacity(sof);
	vtkSmartPointer<vtkVolume> volumeActor = vtkSmartPointer<vtkVolume>::New();
	volumeActor->SetMapper(volumeMapper);
	volumeActor->SetProperty(volumeProperties);
	return volumeActor;
}


