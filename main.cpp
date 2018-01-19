#include "stdafx.h"
#include "loadVolume.h"
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
	}
public:
	vtkActor* cubeActor;
	static myCubeCallback *New() {
		return new myCubeCallback();
	}

	void Execute(vtkObject * caller, unsigned long event, void* calldata) {
		std::cout << "HELLO..." << std::endl;
	}
};

int main(int argc, char** argv) {
	///Carga da imagem
	ObserveLoadProgressCommand::Pointer prog = ObserveLoadProgressCommand::New();
	const std::string txtFile = "C:\\meus dicoms\\Marching Man";//"C:\\meus dicoms\\Abd-Pel w-c  3.0  B30f";/*"C:\\meus dicoms\\Marching Man"*/; //"C:\\meus dicoms\\abdomem-feet-first";//"C:\\meus dicoms\\Marching Man"; //"C:\\meus dicoms\\Marching Man";//
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
	vtkSmartPointer<vtkRenderer> rendererCubeReslicer = vtkSmartPointer<vtkRenderer>::New();
	rendererCubeReslicer->GetActiveCamera()->ParallelProjectionOn();
	rendererCubeReslicer->SetBackground(1, 1, 1);
	vtkSmartPointer<vtkRenderWindow> renderWindowCubeReslicer = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowCubeReslicer->SetPosition(300, 0);
	renderWindowCubeReslicer->AddRenderer(rendererCubeReslicer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractorCubeReslicer = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkInteractorStyleTrackballActor> cubeReslicerStyle = vtkSmartPointer<vtkInteractorStyleTrackballActor>::New();
	renderWindowInteractorCubeReslicer->SetInteractorStyle(cubeReslicerStyle);
	renderWindowCubeReslicer->SetInteractor(renderWindowInteractorCubeReslicer);
	renderWindowInteractorCubeReslicer->Initialize();
	vtkSmartPointer<vtkActor> actorCuboReslice = CreateResliceCube(volumeActor->GetCenter());
	rendererCubeReslicer->AddActor(actorCuboReslice);
	rendererCubeReslicer->ResetCamera();
	vtkSmartPointer<myCubeCallback> cubeCallback = vtkSmartPointer<myCubeCallback>::New();
	rendererCubeReslicer->AddObserver(vtkCommand::EndEvent, cubeCallback);
	cubeCallback->cubeActor = actorCuboReslice;

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
	ctf->AddRGBSegment(1500, 0, 0, 0, 2500, 1, 1, 1);
	vtkSmartPointer<vtkPiecewiseFunction> sof = vtkSmartPointer<vtkPiecewiseFunction>::New();
	sof->AddSegment(1500, 0, 2500, 0.25);
	volumeProperties->SetColor(ctf);
	volumeProperties->SetScalarOpacity(sof);
	vtkSmartPointer<vtkVolume> volumeActor = vtkSmartPointer<vtkVolume>::New();
	volumeActor->SetMapper(volumeMapper);
	volumeActor->SetProperty(volumeProperties);
	return volumeActor;
}


