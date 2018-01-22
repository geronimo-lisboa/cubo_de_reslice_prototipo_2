#include "stdafx.h"
#include "myResliceCube.h"


void myResliceCube::SetEverything()
{
	if (!rendererLayerCubo || !rendererLayerImagem || !imageSource || !boundsDoVolume)
		return;
	//cria a geometria do cubo
	CreateCubeGeometry();
	//Cria o filtro do reslice.
	CreateReslice();
	//Cria o filtro do window/level
	CreateWindowLevelFilter();
	//Cria a geometria que vai exibir o reslice na tela.
	CreateImageGeometry();
	//o callback de quando o cubo é rotacionado
	callbackDeModificacaoDoCubo = vtkSmartPointer<OnAfterRenderCallback>::New();
	callbackDeModificacaoDoCubo->SetOwner(this);
	rendererLayerCubo->AddObserver(vtkCommand::EndEvent, callbackDeModificacaoDoCubo);
}

bool myResliceCube::isInsideBounds(std::array<double, 3> p, double bounds[6]) {
	assert(bounds && "Tem que definir as bounds...");
	if ((bounds[0] < p[0] && p[0] < bounds[1]) &&
		(bounds[2] < p[1] && p[1] < bounds[3]) &&
		(bounds[4] < p[2] && p[2] < bounds[5])) {
		return true;
	}
	else {
		return false;
	}
}

std::pair<std::array<double, 3>, double> myResliceCube::MarchVectorUntilBorder(std::array<double, 3> c, std::array<double, 3> v, double* bounds) {
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

std::array<double, 2> myResliceCube::CalculateResliceExtent(myResliceCube *rc) {
	////qual é o centro?
	const std::array<double, 3> center = { { rc->cubeActor->GetCenter()[0], 
											 rc->cubeActor->GetCenter()[1], 
											 rc->cubeActor->GetCenter()[2], } };
	////qual é o vetor?
	const std::array<double, 3> u = { { rc->cubeActor->GetMatrix()->Element[0][0], 
										rc->cubeActor->GetMatrix()->Element[1][0],
										rc->cubeActor->GetMatrix()->Element[2][0], } };
	const auto uMarch = MarchVectorUntilBorder(center, u, rc->boundsDoVolume);
	const std::array<double, 3> uNeg = { { -rc->cubeActor->GetMatrix()->Element[0][0], 
										   -rc->cubeActor->GetMatrix()->Element[1][0], 
										   -rc->cubeActor->GetMatrix()->Element[2][0], } };
	const auto uNegMarch = MarchVectorUntilBorder(center, uNeg, rc->boundsDoVolume);

	const std::array<double, 3> v = { { rc->cubeActor->GetMatrix()->Element[0][1], 
										rc->cubeActor->GetMatrix()->Element[1][1],
										rc->cubeActor->GetMatrix()->Element[2][1], } };
	const auto vMarch = MarchVectorUntilBorder(center, v, rc->boundsDoVolume);
	const std::array<double, 3> vNeg = { { -rc->cubeActor->GetMatrix()->Element[0][1], 
										   -rc->cubeActor->GetMatrix()->Element[1][1], 
										   -rc->cubeActor->GetMatrix()->Element[2][1], } };
	const auto vNegMarch = MarchVectorUntilBorder(center, vNeg, rc->boundsDoVolume);

	const std::array<double, 3> xVector = { { abs(uMarch.first[0] - uNegMarch.first[0]),
		abs(uMarch.first[1] - uNegMarch.first[1]),
		abs(uMarch.first[2] - uNegMarch.first[2]),
		} };
	const std::array<double, 3> yVector = { { abs(vMarch.first[0] - vNegMarch.first[0]),
		abs(vMarch.first[1] - vNegMarch.first[1]),
		abs(vMarch.first[2] - vNegMarch.first[2]),
		} };
	const double normH = sqrt(xVector[0] * xVector[0] + xVector[1] * xVector[1] + xVector[2] * xVector[2]);
	const double normV = sqrt(yVector[0] * yVector[0] + yVector[1] * yVector[1] + yVector[2] * yVector[2]);
	return{ {normH, normV} };
}

void myResliceCube::CreateReslice()
{
	resliceFilter = vtkSmartPointer<vtkImageSlabReslice>::New();
	resliceFilter->SetInputConnection(imageSource->GetOutputPort());
	resliceFilter->SetOutputSpacing(1, 1, 1);

	resliceMatrix->DeepCopy(cubeActor->GetMatrix());
	resliceFilter->SetResliceAxes(resliceMatrix);

	resliceFilter->SetResliceAxesOrigin(cubeActor->GetCenter()[0], cubeActor->GetCenter()[1], cubeActor->GetCenter()[2]);
	resliceFilter->SetOutputDimensionality(2);
	auto resliceExtent = CalculateResliceExtent(this);
	resliceFilter->SetOutputExtent(0, resliceExtent[0]*1.125, 0, resliceExtent[1]*1.125, 0, 1);
	resliceFilter->Update();

	windowLevelFilter = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
	windowLevelFilter->SetWindow(1000);
	windowLevelFilter->SetLevel(1000);
	windowLevelFilter->SetInputConnection(resliceFilter->GetOutputPort());
	windowLevelFilter->Update();
}

void myResliceCube::CreateCubeGeometry()
{
	vtkSmartPointer<vtkCubeSource> src = vtkSmartPointer<vtkCubeSource>::New();
	src->SetCenter(imageSource->GetOutput()->GetCenter());//O cubo começa no centro da imagem.
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(src->GetOutputPort());
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetRepresentationToWireframe();
	actor->GetProperty()->SetColor(1, 0, 0);
	rendererLayerCubo->AddActor(actor);
	rendererLayerCubo->ResetCamera();
	cubeActor = actor;
}

void myResliceCube::CreateWindowLevelFilter()
{
}

void myResliceCube::CreateImageGeometry()
{
	auto resliceExtent = CalculateResliceExtent(this);//aqui eu tenho o tamanho do extent por ex.:[0, 280, 0,120, 0, 1];
	//O extent é da forma [x0,x1, y0, y1, z0, z1].
	//Os vértices do plano serão da forma v0=[x0,y0,z0] v1=[x1, y0, z0] v2=[x1, y1, z0] v3=[x0, y1, z0].
	std::array<double, 3> v0 = { { 0,0, 0 } };
	std::array<double, 3> v1 = { { resliceExtent[0], 0, 0 } };
	std::array<double, 3> v2 = { { resliceExtent[0], resliceExtent[1], 0 } };
	std::array<double, 3> v3 = { { 0, resliceExtent[1],0 } };
	std::cout << "resliceExtent = " << resliceExtent[0] << ", " << resliceExtent[1] << std::endl;
	//Com esses vértices eu contruo meu polydata
	auto points = vtkSmartPointer<vtkPoints>::New();
	points->InsertNextPoint(v0[0], v0[1], v0[2]);
	points->InsertNextPoint(v1[0], v1[1], v1[2]);
	points->InsertNextPoint(v2[0], v2[1], v2[2]);
	points->InsertNextPoint(v3[0], v3[1], v3[2]);
	auto polygon = vtkSmartPointer<vtkPolygon>::New();
	polygon->GetPointIds()->SetNumberOfIds(4);
	polygon->GetPointIds()->SetId(0,0);
	polygon->GetPointIds()->SetId(1, 1);
	polygon->GetPointIds()->SetId(2, 2);
	polygon->GetPointIds()->SetId(3, 3);
	auto polygons = vtkSmartPointer<vtkCellArray>::New();
	polygons->InsertNextCell(polygon);
	auto polygonPolyData = vtkSmartPointer<vtkPolyData>::New();
	polygonPolyData->SetPoints(points);
	polygonPolyData->SetPolys(polygons);

	auto textureCoordinates = vtkSmartPointer<vtkFloatArray>::New();
	textureCoordinates->SetNumberOfComponents(2);
	float tuple[2] = { 0.0, 0.0  };
	textureCoordinates->InsertNextTuple(tuple);
	tuple[0] = 1.0; tuple[1] = 0.0;  
	textureCoordinates->InsertNextTuple(tuple);
	tuple[0] = 1.0; tuple[1] = 1.0; 
	textureCoordinates->InsertNextTuple(tuple);
	tuple[0] = 0.0; tuple[1] = 1.0; 
	textureCoordinates->InsertNextTuple(tuple);
	polygonPolyData->GetPointData()->SetTCoords(textureCoordinates);
 
	texture = vtkSmartPointer<vtkTexture>::New();
	texture->SetInputConnection(windowLevelFilter->GetOutputPort());

	auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polygonPolyData);

	if (planeActor)
		rendererLayerImagem->RemoveActor(planeActor);
	planeActor = vtkSmartPointer<vtkActor>::New();
	planeActor->SetMapper(mapper);
	planeActor->SetTexture(texture);
	rendererLayerImagem->AddActor(planeActor);
	rendererLayerImagem->ResetCamera();
}

vtkSmartPointer<vtkImageMapToWindowLevelColors> myResliceCube::GetWindowLevelFilter() {
	return windowLevelFilter;
}

myResliceCube::myResliceCube()
{
	resliceFilter = nullptr;
	rendererLayerCubo = nullptr;
	rendererLayerImagem = nullptr;
	imageSource = nullptr;
	cubeActor = nullptr;
	boundsDoVolume = nullptr;
	planeActor = nullptr;
	windowLevelFilter = nullptr;
	resliceFilter = nullptr;
	resliceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	texture = nullptr;
}

void myResliceCube::SetBoundsDoVolume(double * b) {
	boundsDoVolume = b;
	SetEverything();
}

vtkSmartPointer<vtkImageSlabReslice> myResliceCube::GetResliceFilter()
{
	return resliceFilter;
}


void myResliceCube::SetRenderers(vtkRenderer * rc, vtkRenderer * ri)
{
	rendererLayerCubo = rc;
	rendererLayerImagem = ri;
	SetEverything();
}

void myResliceCube::SetSource(vtkImageImport * stc)
{
	imageSource = stc;
	SetEverything();
}

void myResliceCube::OnAfterRenderCallback::SaveDebug()
{
	boost::posix_time::ptime current_date_microseconds = boost::posix_time::microsec_clock::local_time();
	long milliseconds = current_date_microseconds.time_of_day().total_milliseconds();
	std::string filename = "c:\\mprcubov2\\dump\\" + boost::lexical_cast<std::string>(milliseconds) + ".vti";
	vtkSmartPointer<vtkXMLImageDataWriter> debugsave = vtkSmartPointer<vtkXMLImageDataWriter>::New();
	debugsave->SetFileName(filename.c_str());
	debugsave->SetInputConnection(owner->resliceFilter->GetOutputPort());
	debugsave->BreakOnError();
	debugsave->Write();
}

void myResliceCube::OnAfterRenderCallback::Execute(vtkObject * caller, unsigned long event, void * calldata)
{
	//Recalcula o extent do reslice, pega a nova matriz e aplica os dados novos ao reslicer.
	auto newResliceExtent = myResliceCube::CalculateResliceExtent(owner);
	owner->resliceFilter->SetOutputExtent(0, newResliceExtent[0] * 1.125, 0, newResliceExtent[1] * 1.125, 0, 1);
	owner->resliceMatrix->DeepCopy(owner->cubeActor->GetMatrix());
	owner->resliceFilter->SetResliceAxes(owner->resliceMatrix);
	//Executa
	owner->resliceFilter->Update();
	//Aplica o window-level
	owner->windowLevelFilter->Update();
	//refaz a geometria 
	owner->CreateImageGeometry();
	//Salva no hd pra debug
	SaveDebug();
}

void myResliceCube::SetWindowAndLevel(double w, double l) {
	windowLevelFilter->SetWindow(w);
	windowLevelFilter->SetLevel(l);
}