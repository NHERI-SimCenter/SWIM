/*------------------------------------------------------*
| A class to create OpenSees tcl files                  |
| Inputs:  BIM/SAM/EVT/EDP                              |
| Authors: Charles Wang,  UC Berkeley c_w@berkeley.edu  |
|          Frank McKenna, UC Berkeley                   |
| Date:    01/15/2019                                   |
*------------------------------------------------------*/

#include "OpenSeesTclBuilder.h"
#include <jansson.h>
//#include <string.h>
#include <string>
#include <sstream>
#include <map>
#include <stdlib.h> /* atoi */

#include <iostream>
#include <cmath>
#include <typeinfo>
#include <sys/stat.h>


OpenSeesTclBuilder::OpenSeesTclBuilder()
    : filenameBIM(0), filenameSAM(0), filenameEVENT(0), filenameEDP(0),
      filenameTCL(0), filenameUQ(0), analysisType(-1), numSteps(0), dT(0.0), nStory(0)
{
}

OpenSeesTclBuilder::~OpenSeesTclBuilder()
{
  if (filenameBIM != 0)
    delete[] filenameBIM;
  if (filenameSAM != 0)
    delete[] filenameSAM;
  if (filenameEVENT != 0)
    delete[] filenameEVENT;
  if (filenameEDP != 0)
    delete[] filenameEDP;
  if (filenameTCL != 0)
    delete[] filenameTCL;
  if (filenameUQ != 0)
    delete[] filenameUQ;
}

int OpenSeesTclBuilder::writeRV(const char *BIM,
                                  const char *SAM,
                                  const char *EVENT,
                                  const char *outputFilename)
{
  //
  // for now just assume earthquake, dynamic & write the RV
  //

  json_t *root = json_object();
  json_t *randomVariables = json_array();

  json_t *integration = json_object();
  json_t *integrationTypes = json_array();

  json_object_set(integration, "distribution", json_string("discrete_design_set_string"));
  json_object_set(integration, "name", json_string("integration_scheme"));
  json_object_set(integration, "value", json_string("RV.integration_scheme"));
  //    json_array_append(integrationTypes, json_string("NewmarkLinear"));
  //    json_array_append(integrationTypes, json_string("NewmarkAverage"));
  //    json_array_append(integrationTypes, json_string("HHTpt9"));
  json_array_append(integrationTypes, json_string("1"));
  json_array_append(integrationTypes, json_string("2"));
  json_array_append(integrationTypes, json_string("3"));
  json_object_set(integration, "elements", integrationTypes);

  json_array_append(randomVariables, integration);

  json_object_set(root, "RandomVariables", randomVariables);
  // write the file & clean memory
  json_dump_file(root, outputFilename, 0);
  json_object_clear(root);
  return 0;
}



int OpenSeesTclBuilder::createInputFileBeamColumn(const char *BIM,
                                          const char *SAM,
                                          const char *EVENT,
                                          const char *EDP,
                                          const char *tcl)
{
  //
  // make copies of filenames in case methods need them
  //

  if (filenameBIM != 0)
    delete[] filenameBIM;
  if (filenameSAM != 0)
    delete[] filenameSAM;
  if (filenameEVENT != 0)
    delete[] filenameEVENT;
  if (filenameEDP != 0)
    delete[] filenameEDP;
  if (filenameTCL != 0)
    delete[] filenameTCL;
  if (filenameUQ != 0)
    delete[] filenameUQ;

  filenameBIM = (char *)malloc((strlen(BIM) + 1) * sizeof(char));
  filenameSAM = (char *)malloc((strlen(SAM) + 1) * sizeof(char));
  filenameEVENT = (char *)malloc((strlen(EVENT) + 1) * sizeof(char));
  filenameEDP = (char *)malloc((strlen(EDP) + 1) * sizeof(char));
  filenameTCL = (char *)malloc((strlen(tcl) + 1) * sizeof(char));
  filenameUQ = 0;

  strcpy(filenameBIM, BIM);
  strcpy(filenameSAM, SAM);
  strcpy(filenameEVENT, EVENT);
  strcpy(filenameEDP, EDP);
  strcpy(filenameTCL, tcl);



  //
  // find volume of walls
  //
  //Parse BIM Json input file

  json_error_t error0;
  rootBIM = json_load_file((filenameBIM), 0, &error0);
  json_t *GI = json_object_get(rootBIM, "GeneralInformation");
  volumeOfWall = json_number_value(json_object_get(GI, "volume"));

/*
  //
  // set rootEVT
  //
  rootEVENT = json_load_file((filenameEVENT), 0, &error0);
  json_t *EVENTS = json_object_get(rootEVENT, "Events");
  json_t *event = json_array_get(EVENTS,0);
  json_t *verticalLoad = json_array_get(json_object_get(event, "loads"),0);
  double P = json_real_value( json_array_get(json_object_get(verticalLoad, "scales"),0));
*/



  //
  // open geometry tcl script
  //

  printf("making tcl files... \n");

#ifdef Q_OS_WIN
  _mkdir(filenameTCL);
#else
  mkdir(filenameTCL,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  char * basedir = (char *) malloc(1 + strlen(filenameTCL)+ strlen("/staticFiles") );
  strcpy(basedir, filenameTCL);

#ifdef Q_OS_WIN
  _mkdir(strcat(basedir ,"/staticFiles"));
#else
  mkdir(strcat(basedir ,"/staticFiles"),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  std::string filenameTCL_geo = std::string(filenameTCL) + "/staticFiles/modelGeometry.tcl" ; 
  //std::string filenameTCL_geo = "/Users/simcenter/Codes/SimCenter/BIM2SAM/Data/ConcreteShearWallBeamcolumn/For AI-M/DazioWSH6/staticFiles/modelGeometry.tcl";
  //std::cout<< filenameTCL_geo << endl;
  ofstream *s_geo = new ofstream;
  s_geo->open(filenameTCL_geo, ios::out);
  ofstream &tclFile_geo = *s_geo;

  cookGeometry(tclFile_geo);
  s_geo->close();

  //
  // open material tcl script
  //

  std::string filenameTCL_mat = std::string(filenameTCL) + "/staticFiles/modelMaterial.tcl" ;
  //std::string filenameTCL_mat = "/Users/simcenter/Codes/SimCenter/BIM2SAM/Data/ConcreteShearWallBeamcolumn/For AI-M/DazioWSH6/staticFiles/modelMaterial.tcl";
  //std::cout<< filenameTCL_mat << endl;
  ofstream *s_mat = new ofstream;
  s_mat->open(filenameTCL_mat, ios::out);
  ofstream &tclFile_mat = *s_mat;

  cookMaterial(tclFile_mat);
  s_mat->close();

  //
  // open driver tcl script
  //

  //std::string filenameTCL_geo = std::string(filenameTCL) + "_modelGeometry5.tcl";
  std::string filenameTCL_driver = std::string(filenameTCL) + "/wallDriver.tcl" ; //"data/wallDriver.tcl";
  
  //std::cout<< filenameTCL_driver << endl;
  ofstream *s_driver = new ofstream;
  s_driver->open(filenameTCL_driver, ios::out);
  ofstream &tclFile_driver = *s_driver;

  cookDriver(tclFile_driver);
  s_driver->close();
  printf("driver saved at %s \n",filenameTCL_driver.c_str());

/*
  //
  // process the SAM to create the model
  //

  json_error_t error;
  rootSAM = json_load_file(filenameSAM, 0, &error);
  rootSAM = json_object_get(rootSAM, "Structural Analysis Model");

  //mapping = json_object_get(rootSAM, "nodeMapping");

  processNodes(tclFile);

  processMaterials(tclFile);
  processElements(tclFile);
  //processDamping(tclFile);

  rootEVENT = json_load_file(filenameEVENT, 0, &error);
  rootEDP = json_load_file(filenameEDP, 0, &error);

  processEvents(tclFile);

  s->close();
  */
  return 0;
}


int OpenSeesTclBuilder::createInputFile(const char *BIM,
                                          const char *SAM,
                                          const char *EVENT,
                                          const char *EDP,
                                          const char *tcl)
{
  //
  // make copies of filenames in case methods need them
  //
  

  if (filenameBIM != 0)
    delete[] filenameBIM;
  if (filenameSAM != 0)
    delete[] filenameSAM;
  if (filenameEVENT != 0)
    delete[] filenameEVENT;
  if (filenameEDP != 0)
    delete[] filenameEDP;
  if (filenameTCL != 0)
    delete[] filenameTCL;
  if (filenameUQ != 0)
    delete[] filenameUQ;

  filenameBIM = (char *)malloc((strlen(BIM) + 1) * sizeof(char));
  filenameSAM = (char *)malloc((strlen(SAM) + 1) * sizeof(char));
  filenameEVENT = (char *)malloc((strlen(EVENT) + 1) * sizeof(char));
  filenameEDP = (char *)malloc((strlen(EDP) + 1) * sizeof(char));
  filenameTCL = (char *)malloc((strlen(tcl) + 1) * sizeof(char));
  filenameUQ = 0;

  strcpy(filenameBIM, BIM);
  strcpy(filenameSAM, SAM);
  strcpy(filenameEVENT, EVENT);
  strcpy(filenameEDP, EDP);
  strcpy(filenameTCL, tcl);

  //
  // find volume of walls
  //
  //Parse BIM Json input file

  json_error_t error0;
  rootBIM = json_load_file((filenameBIM), 0, &error0);
  json_t *GI = json_object_get(rootBIM, "GeneralInformation");
  volumeOfWall = json_number_value(json_object_get(GI, "volume"));

  //
  // open tcl script
  //

  ofstream *s = new ofstream;
  s->open(filenameTCL, ios::out);
  ofstream &tclFile = *s;

  //
  // process the SAM to create the model
  //

  json_error_t error;
  rootSAM = json_load_file(filenameSAM, 0, &error);
  rootSAM = json_object_get(rootSAM, "Structural Analysis Model");

  mapping = json_object_get(rootSAM, "nodeMapping");

  processNodes(tclFile);

  processMaterials(tclFile);
  processElements(tclFile);
  //processDamping(tclFile);

  rootEVENT = json_load_file(filenameEVENT, 0, &error);
  rootEDP = json_load_file(filenameEDP, 0, &error);

  processEvents(tclFile);

  s->close();
  printf("tcl file for continuum model created ......\n");
  return 0;
}

int OpenSeesTclBuilder::createInputFile(const char *BIM,
                                          const char *SAM,
                                          const char *EVENT,
                                          const char *EDP,
                                          const char *tcl,
                                          const char *UQ)
{
  //
  // make copies of filenames in case methods need them
  //

  if (filenameBIM != 0)
    delete[] filenameBIM;
  if (filenameSAM != 0)
    delete[] filenameSAM;
  if (filenameEVENT != 0)
    delete[] filenameEVENT;
  if (filenameEDP != 0)
    delete[] filenameEDP;
  if (filenameTCL != 0)
    delete[] filenameTCL;
  if (filenameUQ != 0)
    delete[] filenameUQ;

  filenameBIM = (char *)malloc((strlen(BIM) + 1) * sizeof(char));
  filenameSAM = (char *)malloc((strlen(SAM) + 1) * sizeof(char));
  filenameEVENT = (char *)malloc((strlen(EVENT) + 1) * sizeof(char));
  filenameEDP = (char *)malloc((strlen(EDP) + 1) * sizeof(char));
  filenameTCL = (char *)malloc((strlen(tcl) + 1) * sizeof(char));
  filenameUQ = (char *)malloc((strlen(UQ) + 1) * sizeof(char));

  strcpy(filenameBIM, BIM);
  strcpy(filenameSAM, SAM);
  strcpy(filenameEVENT, EVENT);
  strcpy(filenameEDP, EDP);
  strcpy(filenameTCL, tcl);
  strcpy(filenameUQ, UQ);

  //
  // open tcl script
  //

  ofstream *s = new ofstream;
  s->open(filenameTCL, ios::out);
  ofstream &tclFile = *s;

  //
  // process the SAM to create the model
  //

  json_error_t error;
  rootSAM = json_load_file(filenameSAM, 0, &error);

  mapping = json_object_get(rootSAM, "nodeMapping");

  processNodes(tclFile);
  processMaterials(tclFile);
  processElements(tclFile);

  rootEVENT = json_load_file(filenameEVENT, 0, &error);
  rootEDP = json_load_file(filenameEDP, 0, &error);

  processEvents(tclFile);

  s->close();
  return 0;
}

int OpenSeesTclBuilder::processMaterials(ofstream &s)
{
  int index;
  json_t *material;
  json_t *concreteLayer;
  int innerIndex;

  json_t *propertiesObject = json_object_get(rootSAM, "properties");
  json_t *materials = json_object_get(propertiesObject, "uniaxialMaterials");
  json_t *ndMaterials = json_object_get(propertiesObject, "ndMaterials");

  // uniaxialMaterials
  json_array_foreach(materials, index, material)
  {
    const char *type = json_string_value(json_object_get(material, "type"));

    if (strcmp(type, "shear") == 0)
    {
      int tag = json_integer_value(json_object_get(material, "name"));
      double K0 = json_real_value(json_object_get(material, "K0"));
      double Sy = json_real_value(json_object_get(material, "Sy"));
      double eta = json_real_value(json_object_get(material, "eta"));
      double C = json_real_value(json_object_get(material, "C"));
      double gamma = json_real_value(json_object_get(material, "gamma"));
      double alpha = json_real_value(json_object_get(material, "alpha"));
      double beta = json_real_value(json_object_get(material, "beta"));
      double omega = json_real_value(json_object_get(material, "omega"));
      double eta_soft = json_real_value(json_object_get(material, "eta_soft"));
      double a_k = json_real_value(json_object_get(material, "a_k"));
      //s << "uniaxialMaterial Elastic " << tag << " " << K0 << "\n";
      if (K0 == 0)
        K0 = 1.0e-6;
      if (eta == 0)
        eta = 1.0e-6;
      //uniaxialMaterial Hysteretic $matTag $s1p $e1p $s2p $e2p <$s3p $e3p>
      //$s1n $e1n $s2n $e2n <$s3n $e3n> $pinchX $pinchY $damage1 $damage2 <$beta>
      double e2p = Sy / K0 + (alpha - 1) * Sy / eta / K0;
      double e3p = 1.0;
      if (e2p >= 1.0)
        e3p = 10.0 * e2p;

      double e2n = -(beta * Sy / K0 + beta * (alpha - 1) * Sy / eta / K0);
      double e3n = -1.0;
      if (e2n <= -1.0)
        e3n = 10.0 * e2n;

      s << "uniaxialMaterial Hysteretic " << tag << " " << Sy << " " << Sy / K0
        << " " << alpha * Sy << " " << e2p
        << " " << alpha * Sy << " " << e3p
        << " " << -beta * Sy << " " << -beta * Sy / K0 << " " << -beta * (alpha * Sy)
        << " " << e2n
        << " " << -beta * (alpha * Sy) << " " << e3n
        << " " << gamma
        << " " << gamma << " " << 0.0 << " " << 0.0 << " " << a_k << "\n";
    }
    else if (strcmp(type, "Steel01") == 0)
    {
      int tag = json_integer_value(json_object_get(material, "name"));
      double E = json_number_value(json_object_get(material, "E"));
      double fy = json_number_value(json_object_get(material, "fy"));
      double b = json_number_value(json_object_get(material, "b"));

      s << "uniaxialMaterial Steel01 " << tag << " " << fy << " " << E << " " << b << " \n";
    }
  }

  // ndMaterials
  json_array_foreach(ndMaterials, index, material)
  {
    const char *type = json_string_value(json_object_get(material, "type"));

    if (strcmp(type, "PlaneStressRebar") == 0)
    {
      int tag = json_integer_value(json_object_get(material, "name"));
      //char *steel = json_string_value(json_object_get(material, "material"));
      int steel = json_integer_value(json_object_get(material, "material"));
      double angle = json_number_value(json_object_get(material, "angle"));
      //nDMaterial PlaneStressRebarMaterial   tag   $Steel  0; #horizontal
      s << "nDMaterial PlaneStressRebarMaterial " << tag << " " << steel << " " << angle << " \n";
    }
    else if (strcmp(type, "Concrete") == 0)
    {
      int tag = json_integer_value(json_object_get(material, "name"));
      double E = json_number_value(json_object_get(material, "E"));
      double fpc = json_number_value(json_object_get(material, "fpc"));
      double nu = json_number_value(json_object_get(material, "nu"));

      double beta = json_number_value(json_object_get(material, "beta"));
      double Ap = json_number_value(json_object_get(material, "Ap"));
      double An = json_number_value(json_object_get(material, "An"));
      double Bn = json_number_value(json_object_get(material, "Bn"));
      

      //nDMaterial PlasticDamageConcretePlaneStress 11 $E $nu $ft $fc $Beta $Ap $An $Bn;#Web
      s << "set E " << E << ";# ksi \n";
      s << "set nu 0.2 \n";
      s << "set fc [expr " << fpc << "*0.7] ;# ksi \n";
      s << "set ft [expr $fc*0.25] ;# ksi \n";
      /*
      s << "set beta 0.001 \n";
      s << "set Ap 0.4 \n";
      s << "set An 3 \n";
      s << "set Bn 0.8 \n";
      */

      s << "set beta "<< beta <<"  \n";
      s << "set Ap "<< Ap <<" \n";
      s << "set An "<< An <<" \n";
      s << "set Bn "<< Bn <<" \n";
      s << "set outPutFileName resultsOpenSees.out \n";
      

/*
      s << "set beta [lindex $argv 0]  \n";
      s << "set Ap [lindex $argv 1] \n";
      s << "set An [lindex $argv 2] \n";
      s << "set Bn [lindex $argv 3] \n";
      s << "set outPutFileName [lindex $argv 4] \n";
*/

      s << "nDMaterial PlasticDamageConcretePlaneStress " << tag << " $E $nu $ft $fc $beta $Ap $An $Bn ;# concrete \n";
      /*
      s << "nDMaterial PlasticDamageConcretePlaneStress " << tag << " "
        << "37000.0 0.2 2.25 56.0 0.8 0.005 2.0 0.00001"
        //<< "3700.0 0.2 2.25 56.0 0.8 0.005 2.0 0.00001"
        << " ;# tag E fpc nu. I have no idea which material should be used.\n";
        */
    }
    else if (strcmp(type, "LayeredConcrete") == 0)
    {
      int tag = json_integer_value(json_object_get(material, "name"));
      json_t *concreteLayers = json_object_get(material, "layers");
      int numLayers = int(json_array_size(concreteLayers));
      //nDMaterial PlaneStressLayeredMaterial 2 3 11 $t 12 [expr $t*.005] 13 [expr $t*.0044*1.17];# Web
      s << "nDMaterial PlaneStressLayeredMaterial " << tag << " " << numLayers << " ";
      json_array_foreach(concreteLayers, innerIndex, concreteLayer)
      {
        int layerMaterial = json_integer_value(json_object_get(concreteLayer, "material"));
        double layerThickness = json_number_value(json_object_get(concreteLayer, "thickness"));
        s << layerMaterial << " " << layerThickness << " ";
      }
      s << " \n";
    }
  }

  s << "\n";
  return 0;
}

int OpenSeesTclBuilder::processSections(ofstream &s)
{
  return 0;
}


int OpenSeesTclBuilder::cookGeometry(ofstream &s)
{
  size_t index;
  json_t *node;
  json_t *elements;
  json_t *section;

  json_t *properties;
  json_t *sections;

  json_t *geometry = json_object_get(rootSAM, "geometry");
  nodes = json_object_get(geometry, "nodes");
  elements = json_object_get(geometry, "elements");
  properties = json_object_get(rootSAM, "properties");
  sections = json_object_get(properties, "sections");




  NDM = 0;
  NDF = 0;

  json_array_foreach(nodes, index, node)
  {

    int tag = json_integer_value(json_object_get(node, "name"));
    if (nStory < tag)
      nStory = tag;
    json_t *crds = json_object_get(node, "crd");
    int sizeCRD = json_array_size(crds);
    int ndf = json_integer_value(json_object_get(node, "ndf"));

    if (sizeCRD != NDM || ndf != NDF)
    {
      NDM = sizeCRD;
      NDF = ndf;
      // issue new model command if node size changes
      //s << "model BasicBuilder -ndm " << NDM << " -ndf " << NDF << "\n";
    }

    s << "node " << tag << " ";
    json_t *crd;
    int crdIndex;
    json_array_foreach(crds, crdIndex, crd)
    {
      s << json_number_value(crd) << " ";
      //s << json_real_value(crd)*2.54/100 << " ";// convert to m
    }
    s << "\n";
  }

  // fix base
  s << "fix 1 1 1 1;" << "\n";


  // sections
  
  json_array_foreach(sections, index, section)
  {
    std::string sectionType = json_string_value(json_object_get(section, "type"));

if (!sectionType.compare("FiberSection2d"))
{
    double thickness = getThickness(rootBIM, int(0));
    s << "section fiberSec "<< index+1 <<"     {; \n";
    std::string sectionName = json_string_value(json_object_get(section, "name"));
    
    json_t *fibers = json_object_get(section, "fibers");
    size_t index_fiber;
    json_t *fiber;
    json_array_foreach(fibers, index_fiber, fiber)
    {
      s << "patch rect ";

      json_t *fiberCoord = (json_object_get(fiber, "coord"));
      double y0 = json_number_value(json_array_get(fiberCoord,0));
      double fiberArea = json_number_value(json_object_get(fiber, "area"));
      std::string fiberMater = json_string_value(json_object_get(fiber, "material"));
      s << fiberMater << " ";
      s << "1 1 ";
      double width_t = fiberArea/thickness;
      double yi = y0 - width_t/2.0;
      double zi = -thickness/2.0;
      double yj = y0 + width_t/2.0;
      double zj = thickness/2.0;
      s << yi << " " << zi << " " << yj << " " << zj << " \n";

    }
    s << "};\n";
}

if (!sectionType.compare("SectionAggregator"))
{
  
  //{"name": "101", "type": "SectionAggregator", "section": "1", "materials": ["90"], "dof": ["Vy"]},
    s << "section Aggregator ";
    std::string sectionName = json_string_value(json_object_get(section, "name"));
    std::string sectionSection = json_string_value(json_object_get(section, "section"));
    std::string sectionMaterial = json_string_value(json_array_get((json_object_get(section, "materials")),0));
    std::string sectionDof = json_string_value(json_array_get(json_object_get(section, "dof"),0));
    
    s << sectionName << " " << sectionMaterial << " " << sectionDof << " -section " << sectionSection << " \n";
}
    
    
  }

  // create element
  //element forceBeamColumn 1 2 3 5 -sections  101 102 103 102 101 1 -iter 50 1e-8;
  //element forceBeamColumn 2 3 4 5 -sections  101 102 103 102 101 1 -iter 50 1e-8;

  json_t *element;
  json_t *eleSect;
  size_t index_eleSects;
    json_array_foreach(elements, index, element)
  {
    int eleName = json_integer_value(json_object_get(element, "name"));
    std::string eleType = json_string_value(json_object_get(element, "type"));
    
    if (!eleType.compare("ForceBeamColumn2d"))
    {
      s << "element forceBeamColumn ";
      s << eleName << " ";
      int nodei = json_integer_value(json_array_get((json_object_get(element, "nodes")),0));
      int nodej = json_integer_value(json_array_get((json_object_get(element, "nodes")),1));
      std::string eleTrans = json_string_value(json_object_get(element, "crdTransformation"));
      
      s << nodei << " " << nodej << " ";
      
      json_t *eleSections = json_object_get(element, "sections");
      s << json_array_size(eleSections) << " ";
      s << "-sections ";
      json_array_foreach(eleSections,index_eleSects,eleSect){
        s << json_string_value(json_array_get(eleSections, index_eleSects)) << " ";
      }
      s << eleTrans << " ";
      s << "-iter 50 1e-8 \n";
    }

    if (!eleType.compare("ZeroLength"))
    {
      //element zeroLength 197 1 2 -mat 99 -dir 6;
      //element zeroLength $eleTag $iNode $jNode -mat $matTag1 $matTag2 ... -dir $dir1 $dir2 ...<-doRayleigh $rFlag> <-orient $x1 $x2 $x3 $yp1 $yp2 $yp3>
      s << "element zeroLength ";
      s << eleName << " ";
      int nodei = json_integer_value(json_array_get((json_object_get(element, "nodes")),0));
      int nodej = json_integer_value(json_array_get((json_object_get(element, "nodes")),1));
      std::string eleMat = json_string_value(json_array_get((json_object_get(element, "materials")),0));

      std::string  dirString = json_string_value(json_array_get((json_object_get(element, "dof")),0));
      std::string dir;
      if (!dirString.compare("Mz"))
        dir = "6";
      else if (!dirString.compare("Vy"))
        dir = "2";
      else if (!dirString.compare("P"))
        dir = "1";
      else
        return 1;
      

      s << nodei << " " << nodej << " " << "-mat " << eleMat << " " << "-dir " << dir << "\n";

    }

    

  }




  

  return 0;
}

int OpenSeesTclBuilder::cookMaterial(ofstream &s)
{
  size_t index;
  json_t *uniaxialMat;

  json_t *properties;
  json_t *uniaxialMaterials;

  properties = json_object_get(rootSAM, "properties");
  uniaxialMaterials = json_object_get(properties, "uniaxialMaterials");


  json_array_foreach(uniaxialMaterials, index, uniaxialMat)
  {
    std::string matType = json_string_value(json_object_get(uniaxialMat, "type"));
    if (!matType.compare("Concrete02"))
    {
      //uniaxialMaterial Concrete02 $matTag $fpc $epsc0 $fpcu $epsU $lambda $ft $Ets
      std::string matName = json_string_value(json_object_get(uniaxialMat, "name"));
      double fpc = json_number_value(json_object_get(uniaxialMat, "fc"));
      double epsc0 = json_number_value(json_object_get(uniaxialMat, "epsc"));
      double fpcu = json_number_value(json_object_get(uniaxialMat, "fcu"));
      double epscu = json_number_value(json_object_get(uniaxialMat, "epscu"));
      double lambda = json_number_value(json_object_get(uniaxialMat, "ratio"));
      double ft = json_number_value(json_object_get(uniaxialMat, "ft"));
      double Ets = json_number_value(json_object_get(uniaxialMat, "Ets"));
      s << "uniaxialMaterial Concrete02 ";
      s << matName << " " << fpc << " " << epsc0 << " " << fpcu << " ";
      s << epscu << " " << lambda << " " << ft << " " << Ets << " \n";

    }

    if (!matType.compare("Steel02"))
    {
      //uniaxialMaterial Steel02 $matTag $Fy $E $b $R0 $cR1 $cR2 <$a1 $a2 $a3 $a4 $sigInit>
      std::string matName = json_string_value(json_object_get(uniaxialMat, "name"));
      double Fy = json_number_value(json_object_get(uniaxialMat, "fy"));
      double EE = json_number_value(json_object_get(uniaxialMat, "E"));
      double b = json_number_value(json_object_get(uniaxialMat, "b"));
      double R0 = json_number_value(json_object_get(uniaxialMat, "R0"));
      double cR1 = json_number_value(json_object_get(uniaxialMat, "cR1"));
      double cR2 = json_number_value(json_object_get(uniaxialMat, "cR2"));
      
      s << "uniaxialMaterial Steel02 ";
      s << matName << " " << Fy << " " << EE << " " << b << " ";
      s << R0 << " " << cR1 << " " << cR2 <<  " \n";

    }

    if (!matType.compare("MinMaxMaterial"))
    {
      //uniaxialMaterial MinMax $matTag $otherTag <-min $minStrain> <-max $maxStrain>
      std::string matName = json_string_value(json_object_get(uniaxialMat, "name"));
      std::string otherMat = json_string_value(json_object_get(uniaxialMat, "material"));
      double epsMin = json_number_value(json_object_get(uniaxialMat, "epsMin"));
      double epsMax = json_number_value(json_object_get(uniaxialMat, "epsMax"));

      s << "uniaxialMaterial MinMax ";
      s << matName << " " << otherMat << " -min " << epsMin << " -max " << epsMax << " \n";

    }

    if (!matType.compare("Steel01"))
    {
      //uniaxialMaterial Steel01 $matTag $Fy $E0 $b <$a1 $a2 $a3 $a4>
      std::string matName = json_string_value(json_object_get(uniaxialMat, "name"));
      double Fy = json_number_value(json_object_get(uniaxialMat, "fy"));
      double EE = json_number_value(json_object_get(uniaxialMat, "E"));
      double b = json_number_value(json_object_get(uniaxialMat, "b"));

      s << "uniaxialMaterial Steel01 ";
      s << matName << " " << Fy << " " << EE << " " << b << " \n";
    }

    if (!matType.compare("ElasticMaterial"))
    {
      //uniaxialMaterial Elastic $matTag $E <$eta> <$Eneg>
      //"Epos": 1e+12, "Eneg": 1e+12, "eta": 0}
      std::string matName = json_string_value(json_object_get(uniaxialMat, "name"));
      double Eneg = json_number_value(json_object_get(uniaxialMat, "Eneg"));
      //double EE = json_number_value(json_object_get(uniaxialMat, "E"));


      s << "uniaxialMaterial Elastic ";
      s << matName <<  " " << Eneg << " \n";
    }


    
    
  }


  s << "geomTransf Corotational 1;\n";

  return 0;
}


int OpenSeesTclBuilder::cookDriver(ofstream &s)
{
  

  json_t *geometry = json_object_get(rootSAM, "geometry");
  json_t *elements = json_object_get(geometry, "elements");
  json_t *element;
  size_t index;

  int nElem_forceBeamColumn = 0 ;
  json_array_foreach(elements, index, element)
  {
    std::string eleType = json_string_value(json_object_get(element, "type"));
    if (!eleType.compare("ForceBeamColumn2d"))
    {
        nElem_forceBeamColumn += 1;
    }
  }

  double P = getVerticalLoad();
  double mtop = getMoment();
  vector<double> peaks;
  getPeaks(peaks);

  //std::cout << "peaks' size : " << peaks.size() << std::endl;
  


  std::string wallName = "shearWall";


  s << "set pid [getPID] \n";
  s << "set numP [getNP] \n";
  s << "logFile " << wallName << "log.txt \n";
  s << "set count 0 \n\n";

  s << "foreach nInt {5} { \n";
  s << "  if {[expr $count % $numP] == $pid}  { \n";
  s << "  model BasicBuilder -ndm 2 -ndf 3; \n";
  s << "  metaData -title \"RCWall_" <<"\" -description \"distributed plasticity frame model of RC wall\" -contact \"NHERI SimCenter\"; \n";
  s << "  defaultUnits -force kip -length in -time sec -temp F; \n";
  s << "  set outName [format \""<<"outputs"<<"\" $nInt]; \n";
  s << "  file mkdir $outName; \n";
  s << "  set geomName [format \"staticFiles/modelGeometry.tcl\" $nInt]; \n";
  s << "  set matName [format \"staticFiles/modelMaterial.tcl\" $nInt]; \n";
  s << "  source $matName; \n";
  s << "  source $geomName; \n";
  s << "  set nElem " << nElem_forceBeamColumn << " \n";
  s << "  set nTop [expr $nElem + 2]; \n";
  s << "  recorder Node -file $outName/RBase.out -node 1 -dof 1 2 3 reaction; \n";
  s << "  recorder Node -file $outName/Dtop.out -node $nTop -dof 1 2 3 disp; \n";
  s << "  print -JSON -file RCWall_${outName}_SAM_reMade.json \n";
  s << "  pattern Plain 1 Linear {load $nTop 0.0 "<< P <<" 0.0}; \n";
  

  s << "  constraints Plain; \n";
  s << "  numberer Plain; \n";
  s << "  system BandGeneral; \n";
  s << "  test NormUnbalance 1e-6 75;  \n";
  s << "  algorithm Newton; \n";
  s << "  integrator LoadControl 0.1; \n";
  s << "  analysis Static; \n";
  s << "  analyze 10; \n";
  s << "  loadConst -time 0.0; \n";
  s << "  pattern Plain 200 Linear {load $nTop 1.0 0.0 "<< -mtop <<"}; \n";
  s << "  set nodeTag $nTop;set dofTag 1;set Dtot 0;set stepCount 1; \n";
  s << "  set Peak [list ";

  for (int i=0; i<peaks.size(); i++)
    s << peaks[i] << " ";

  s << " ]; \n";
  s << "  source CyclicSolutionAlgorithm.tcl; \n";
  s << "  wipeAnalysis;\n";
  s << "  wipe; \n";
  s << "}\n";
  s << "incr count 1 \n";
  s << "} \n";
  s << "exit \n";
  s << " \n";
     
  
     
     
     
     
     
     
     
     


  return 0;
}

int OpenSeesTclBuilder::processNodes(ofstream &s)
{
  size_t index;
  json_t *node;

  json_t *geometry = json_object_get(rootSAM, "geometry");
  nodes = json_object_get(geometry, "nodes");

  NDM = 0;
  NDF = 0;

  json_array_foreach(nodes, index, node)
  {

    int tag = json_integer_value(json_object_get(node, "name"));
    if (nStory < tag)
      nStory = tag;
    json_t *crds = json_object_get(node, "crd");
    int sizeCRD = json_array_size(crds);
    int ndf = json_integer_value(json_object_get(node, "ndf"));

    if (sizeCRD != NDM || ndf != NDF)
    {
      NDM = sizeCRD;
      NDF = ndf;
      // issue new model command if node size changes
      s << "model BasicBuilder -ndm " << NDM << " -ndf " << NDF << "\n";
    }

    s << "node " << tag << " ";
    json_t *crd;
    int crdIndex;
    json_array_foreach(crds, crdIndex, crd)
    {
      s << json_number_value(crd) << " ";
      //s << json_real_value(crd)*2.54/100 << " ";// convert to m
    }

    json_t *mass = json_object_get(node, "mass");
    if (mass != NULL)
    {
      s << "-mass ";
      double massV = json_number_value(mass);
      for (int i = 0; i < NDF; i++)
        s << massV << " ";
    }

    s << "\n";
  }
  /*
  int nodeTag = getNode(1, 1);
  s << "fix " << nodeTag;
  for (int i = 0; i < NDF; i++)
    s << " " << 1;
  s << "\n\n";
*/

  return 0;
}

int OpenSeesTclBuilder::processElements(ofstream &s)
{
  int index;
  json_t *element;

  json_t *geometry = json_object_get(rootSAM, "geometry");
  //json_t *elements = json_object_get(geometry, "elements");
  elements = json_object_get(geometry, "elements");

  json_array_foreach(elements, index, element)
  {

    int tag = json_integer_value(json_object_get(element, "name"));
    const char *type = json_string_value(json_object_get(element, "type"));
    if (strcmp(type, "shear_beam") == 0)
    {
      s << "element zeroLength " << tag << " ";
      json_t *nodesInElement = json_object_get(element, "nodes");
      json_t *nodeTag;
      int nodeIndex;
      json_array_foreach(nodesInElement, nodeIndex, nodeTag)
      {
        s << json_integer_value(nodeTag) << " ";
      }

      int matTag = json_integer_value(json_object_get(element, "uniaxial_material"));
      s << "-mat " << matTag << " -dir 1 \n";
    }
    else if (strcmp(type, "shear_beam2d") == 0)
    {
      s << "element zeroLength " << tag << " ";
      json_t *nodesInElement = json_object_get(element, "nodes");
      json_t *nodeTag;
      int nodeIndex;
      json_array_foreach(nodesInElement, nodeIndex, nodeTag)
      {
        s << json_integer_value(nodeTag) << " ";
      }

      int matTag = json_integer_value(json_object_get(element, "uniaxial_material"));
      s << "-mat " << matTag << " " << matTag << " -dir 1 2\n";
    }
    else if (strcmp(type, "FourNodeQuad") == 0)
    {
      s << "element quad " << tag << " ";
      json_t *nodesInElement = json_object_get(element, "nodes");
      json_t *nodeTag;
      int nodeIndex;
      json_array_foreach(nodesInElement, nodeIndex, nodeTag)
      {
        s << json_integer_value(nodeTag) << " ";
      }

      double quadThick = 1.0; // thickness of quad element
      std::string quadType = "PlaneStress";

      int matTag = json_integer_value(json_object_get(element, "material"));
      s << quadThick << " " << quadType << " " << matTag << "\n";
    }
  }
  s << "\n";
  return 0;
}

int OpenSeesTclBuilder::processDamping(ofstream &s) // not doing this for shear walls
{
  json_t *propertiesObject = json_object_get(rootSAM, "properties");
  double damping = json_number_value(json_object_get(propertiesObject, "dampingRatio"));
  s << "set xDamp " << damping << ";\n"
    << "set MpropSwitch 1.0;\n"
    << "set KcurrSwitch 0.0;\n"
    << "set KinitSwitch 0.0;\n"
    << "set KcommSwitch 1.0;\n"
    << "set nEigenI 1;\n";

  json_t *geometry = json_object_get(rootSAM, "geometry");
  //json_t *nodes = json_object_get(geometry, "nodes");
  int nStory = json_array_size(nodes) - 1;
  int nEigenJ = 0;
  if (nStory <= 2)
    nEigenJ = nStory * 2; //first mode or second mode
  else
    nEigenJ = 3 * 2; //third mode

  s << "set nEigenJ " << nEigenJ << ";\n"
    << "set lambdaN [eigen -fullGenLapack " << nStory * 2 << "];\n"
    << "set lambdaI [lindex $lambdaN [expr $nEigenI-1]];\n"
    << "set lambdaJ [lindex $lambdaN [expr $nEigenJ-1]];\n"
    << "set omegaI [expr pow($lambdaI,0.5)];\n"
    << "set omegaJ [expr pow($lambdaJ,0.5)];\n"
    << "set alphaM [expr $MpropSwitch*$xDamp*(2*$omegaI*$omegaJ)/($omegaI+$omegaJ)];\n"
    << "set betaKcurr [expr $KcurrSwitch*2.*$xDamp/($omegaI+$omegaJ)];\n"
    << "set betaKinit [expr $KinitSwitch*2.*$xDamp/($omegaI+$omegaJ)];\n"
    << "set betaKcomm [expr $KcommSwitch*2.*$xDamp/($omegaI+$omegaJ)];\n"
    << "rayleigh $alphaM $betaKcurr $betaKinit $betaKcomm;\n";
  return 0;
}

int OpenSeesTclBuilder::processEvents(ofstream &s)
{

  //
  //   foreach EVENT
  //   create load pattern
  //   add recorders
  //   add analysis script
  //

  int numTimeSeries = 1;
  int numPatterns = 1;

  int index;
  json_t *event;

  json_t *events = json_object_get(rootEVENT, "Events");
  json_t *edps = json_object_get(rootEDP, "EngineeringDemandParameters");

  int numEvents = json_array_size(events);
  int numEDPs = json_array_size(edps);

  stringstream myRecorderStr;

  int nodeTag;
  int indexLoadInfo;
  double startPoint[2];
  double endPoint[2];

  for (int i = 0; i < numEvents; i++)
  {

    // process event
    json_t *event = json_array_get(events, i);
    processEvent(s, event, numPatterns, numTimeSeries);
    const char *eventName = json_string_value(json_object_get(event, "name"));

    // create recorder foreach EDP
    // loop through EDPs and find corresponding EDP
    for (int j = 0; j < numEDPs; j++)
    {

      json_t *eventEDPs = json_array_get(edps, j);
      const char *edpEventName = json_string_value(json_object_get(eventEDPs, "name"));

      if (strcmp(edpEventName, eventName) == 0)
      {
        json_t *eventEDP = json_object_get(eventEDPs, "responses");
        int numResponses = json_array_size(eventEDP);
        for (int k = 0; k < numResponses; k++)
        {
          json_t *response = json_array_get(eventEDP, k);
          string type(json_string_value(json_object_get(response, "type")));
          string loadName(json_string_value(json_object_get(response, "loadName")));

          //starthere
          string responseType;
          const char *positionType = json_string_value(json_object_get(response, "positionType")); // should be empty, for now
          const char *time = json_string_value(json_object_get(response, "time"));
          json_t *positions = json_object_get(response, "positions");
          json_t *responseData = json_object_get(response, "data");

          // get response type: Force, Displacement, Velocity, Acceleration
          if (type.find("Force") != string::npos || type.find("Shear") != string::npos)
            responseType = "Force";
          else if (type.find("Displacement") != string::npos)
            responseType = "Displacement";
          else if (type.find("Velocity") != string::npos)
            responseType = "Velocity";
          else if (type.find("Acceleration") != string::npos)
            responseType = "Acceleration";
          else
            printf("responseType is not found! \n");

          // get position type: Point, Line, Face...
          if (type.find("Point") != string::npos)
            positionType = "Point";
          else if (type.find("Line") != string::npos)
            positionType = "Line";
          else if (type.find("Floor") != string::npos)
            positionType = "Floor";
          else
            printf("positionType is not found! \n");

          //int numNodesOnLine = 0;
          //int nodesOnline[100000] = {-1};
          if (strcmp(positionType, "Line") == 0 || strcmp(positionType, "Floor") == 0)
          {
            for (int i = 0; i < json_array_size(positions); i++)
            {

              int numNodesOnLine = 0;
              int nodesOnline[100000] = {-1};

              json_t *position = json_array_get(positions, i);
              json_t *value = json_array_get(responseData, i);
              size_t indextmpthis;
              json_t *valuetmpthis;
              double valueTmpDouble;
              json_array_foreach(value, indextmpthis, valuetmpthis) {
                  if(json_is_real(valuetmpthis))
                      valueTmpDouble = json_real_value(valuetmpthis);
                  if(json_is_integer(valuetmpthis))
                      valueTmpDouble = double(json_integer_value(valuetmpthis));
                  forceVec.push_back(valueTmpDouble);
              }

              json_t *startPointJson = json_object();
              json_t *endPointJson = json_object();
              string dof;
              if (strcmp(positionType, "Floor") == 0)
              {
                const char *floorStr = json_string_value(json_object_get(position, "floor"));

                //startPoint = json_object();
                json_object_set(startPointJson, "cline", json_string("1")); // TODO: need to consider more complicated structures
                json_object_set(startPointJson, "floor", json_string(floorStr));
                //endPoint = json_object();
                json_object_set(endPointJson, "cline", json_string("2")); // TODO: need to consider more complicated structures
                json_object_set(endPointJson, "floor", json_string(floorStr));

                dof = (json_string_value(json_object_get(position, "dof")));
              }
              else if (strcmp(positionType, "Line") == 0)
              {
                startPointJson = json_object_get(position, "startPoint");
                endPointJson = json_object_get(position, "endPoint");

                dof = (json_string_value(json_object_get(position, "dof")));
              }

              int startPointCline = atoi(json_string_value(json_object_get(startPointJson, "cline")));
              int startPointFloor = atoi(json_string_value(json_object_get(startPointJson, "floor")));
              int endPointCline = atoi(json_string_value(json_object_get(endPointJson, "cline")));
              int endPointFloor = atoi(json_string_value(json_object_get(endPointJson, "floor")));
              /*
              json_t *infoDataArray = json_object_get(info, "data");
              const char *dof = json_string_value(json_object_get(json_array_get(infoDataArray, 0), "dof"));
              json_t *value = json_object_get(json_array_get(infoDataArray, 0), "value");
              */

              size_t indexInfoData;
              json_t *infoData;

              int startNodeTag = getNode(startPointCline, startPointFloor);
              int endNodeTag = getNode(endPointCline, endPointFloor);

              int findStartPoint = getNodeCrdByTag(startNodeTag, startPoint);
              int findEndPoint = getNodeCrdByTag(endNodeTag, endPoint);

              if (findStartPoint == 1)
              {
                //printf("startPoint tag %d \n", startNodeTag);
                //printf("startPoint x %f \n", startPoint[0]);
                //printf("startPoint y %f \n", startPoint[1]);
              }
              if (findEndPoint == 1)
              {
                //printf("endPoint tag %d \n", endNodeTag);
                //printf("endPoint x %f \n", endPoint[0]);
                //printf("endPoint y %f \n", endPoint[1]);
              }

              numNodesOnLine = findNodesOnStraigntLine(startPoint, endPoint, nodesOnline);
              if (numNodesOnLine > 0)
              {
                //printf("Nodes on line, 1st: %d \n", nodesOnline[0]);

                for (int nodeIndex = 0; nodeIndex < 10000; nodeIndex++)
                {
                  if (nodesOnline[nodeIndex] > 0)
                  {
                    nodeTag = nodesOnline[nodeIndex];
                    myRecorderStr << "lappend EDPNodes " << nodeTag << "\n";
                  }
                  else
                  {
                    break;
                  }
                }
                if (responseType.compare("Force") == 0)
                  myRecorderStr << "eval \"recorder Node -file $outPutFileName -node $EDPNodes -dof 1 reaction\"\n";
                else if (responseType.compare("Displacement") == 0)
                  myRecorderStr << "eval \"recorder Node -file $outPutFileName -node $EDPNodes -dof 1 disp\"\n";
                else if (responseType.compare("Velocity") == 0)
                  myRecorderStr << "eval \"recorder Node -file $outPutFileName -node $EDPNodes -dof 1 vel\"\n";
                else if (responseType.compare("Acceleration") == 0)
                  myRecorderStr << "eval \"recorder Node -file $outPutFileName -node $EDPNodes -dof 1 accel\"\n";
              }
            }
          }
        }
        //endhere

        /*
          if (strcmp(type, "force") == 0)
          {

            int cline = atoi(json_string_value(json_object_get(response, "cline")));
            int floor = atoi(json_string_value(json_object_get(response, "floor")));
            int dof = atoi(json_string_value(json_object_get(response, "dof")));

            int nodeTag = this->getNode(cline, floor);
            //	    std::ostringstream fileString(string(edpEventName)+string(type));
            string fileString;
            ostringstream temp; //temp as in temporary
            temp << filenameBIM << edpEventName << "." << type << "." << cline << "." << floor << ".out";
            fileString = temp.str();
            const char *fileName = fileString.c_str();

            int startTimeSeries = numTimeSeries - NDF;
            
            //s << "recorder Node -file " << fileName;
            //s << " -node " << nodeTag << " -dof ";
            //s << dof << " ";
            //s << " reaction\n";
            

            myRecorderStr << "recorder Node -file resultsOpenSees.out";
            myRecorderStr << " -node " << nodeTag << " -dof ";
            myRecorderStr << dof << " ";
            myRecorderStr << " reaction\n";
          }
          */
      }

      if (strcmp(edpEventName, eventName) == 1000) // 0 This is old code, useful to earthquake events.
      {
        json_t *eventEDP = json_object_get(eventEDPs, "responses");
        int numResponses = json_array_size(eventEDP);
        for (int k = 0; k < numResponses; k++)
        {

          json_t *response = json_array_get(eventEDP, k);
          const char *type = json_string_value(json_object_get(response, "type"));
          if (strcmp(type, "max_abs_acceleration") == 0)
          {
            int cline = json_integer_value(json_object_get(response, "cline"));
            int floor = json_integer_value(json_object_get(response, "floor"));

            int nodeTag = this->getNode(cline, floor);
            //	    std::ostringstream fileString(string(edpEventName)+string(type));
            string fileString;
            ostringstream temp; //temp as in temporary
            temp << filenameBIM << edpEventName << "." << type << "." << cline << "." << floor << ".out";
            fileString = temp.str();

            const char *fileName = fileString.c_str();

            int startTimeSeries = numTimeSeries - NDF;
            s << "recorder EnvelopeNode -file " << fileName;
            s << " -timeSeries ";
            for (int i = 0; i < NDF; i++)
              s << i + startTimeSeries << " ";
            s << " -node " << nodeTag << " -dof ";
            for (int i = 1; i <= NDF; i++)
              s << i << " ";
            s << " accel\n";
          }

          else if (strcmp(type, "max_drift") == 0)
          {
            int cline = json_integer_value(json_object_get(response, "cline"));
            int floor1 = json_integer_value(json_object_get(response, "floor1"));
            int floor2 = json_integer_value(json_object_get(response, "floor2"));

            int nodeTag1 = this->getNode(cline, floor1);
            int nodeTag2 = this->getNode(cline, floor2);

            string fileString1;
            string fileString2;
            ostringstream temp1; //temp as in temporary
            ostringstream temp2; //temp as in temporary
            temp1 << filenameBIM << edpEventName << "." << type << "." << cline << "." << floor1 << "." << floor2 << "-1.out";
            temp2 << filenameBIM << edpEventName << "." << type << "." << cline << "." << floor1 << "." << floor2 << "-2.out";
            fileString1 = temp1.str();
            fileString2 = temp2.str();

            const char *fileName1 = fileString1.c_str();
            const char *fileName2 = fileString2.c_str();

            s << "recorder EnvelopeDrift -file " << fileName1;
            s << " -iNode " << nodeTag1 << " -jNode " << nodeTag2;
            s << " -dof 1 -perpDirn 1\n";

            s << "recorder EnvelopeDrift -file " << fileName2;
            s << " -iNode " << nodeTag1 << " -jNode " << nodeTag2;
            s << " -dof 2 -perpDirn 1\n";
          }

          else if (strcmp(type, "residual_disp") == 0)
          {

            int cline = json_integer_value(json_object_get(response, "cline"));
            int floor = json_integer_value(json_object_get(response, "floor"));

            int nodeTag = this->getNode(cline, floor);

            string fileString;
            ostringstream temp; //temp as in temporary
            temp << filenameBIM << edpEventName << "." << type << "." << cline << "." << floor << ".out";
            fileString = temp.str();

            const char *fileName = fileString.c_str();

            s << "recorder Node -file " << fileName;
            s << " -node " << nodeTag << " -dof ";
            for (int i = 1; i <= NDF; i++)
              s << i << " ";
            s << " disp\n";
          }
        }
      }
    }

    // create analysis
    if (analysisType == -999) // 1
    {
      //      s << "handler Plain\n";
      s << "numberer RCM\n";
      s << "system BandGen\n";

      if (filenameUQ != 0)
      {
        printf("HI filenameUQ %s\n", filenameUQ);
        json_error_t error;
        json_t *rootUQ = json_load_file(filenameUQ, 0, &error);
        json_dump_file(rootUQ, "TEST", 0);

        json_t *theRVs = json_object_get(rootUQ, "RandomVariables");
        json_t *theRV;
        int index;

        json_array_foreach(theRVs, index, theRV)
        {
          const char *type = json_string_value(json_object_get(theRV, "name"));
          //printf("type: %s\n", type);
          if (strcmp(type, "integration_scheme") == 0)
          {
            //	    const char *typeI = json_string_value(json_object_get(theRV,"value"));
            int typeI = json_integer_value(json_object_get(theRV, "value"));
            if (typeI == 1)
            {
              s << "integrator Newmark 0.5 0.25\n";
            }
            else if (typeI == 2)
            {
              s << "integrator Newmark 0.5 0.1667\n";
            }
            else if (typeI == 3)
            {
              s << "integrator HHT .9\n";
            }
            //	  printf("type: %s\n",typeI);
            /*
	    int typeI = json_integer_value(json_object_get(theRV,"value"));
	    if (strcmp(typeI,"NewmarkLinear") == 0) {    
	      //	      s << "integrator Newmark 0.5 0.25\n";
	    } else if (strcmp(typeI,"NewmarkAverage") == 0) {    
	      //	      s << "integrator Newmark 0.5 0.1667\n";
	    } else if (strcmp(typeI,"HHTpt9") == 0) {    
	      //	      s << "integrator HHT .9\n";
	    }
	    */
          }
        }
      }

      s << "analysis Transient\n";
      s << "analyze " << numSteps << " " << dT << "\n";
    }

    if (analysisType == 100) // 1  gravity
    {
      s << "constraints Plain\n";
      s << "system UmfPack\n";
      s << "test NormDispIncr 1.0e-3 30\n";
      s << "algorithm ModifiedNewton\n";
      s << "numberer RCM\n";
      s << "integrator LoadControl 1\n";
      s << "analysis Static\n";
      s << "analyze " << 1 << "\n";
    }

    // add self weihts
    //pattern Plain 100 "Linear" {
    //  load $nodeTag 0.0 [expr ($widthX*$heightY*$thickness*$rho*$g)/(($numX+1)*($numY+1))];#SELF-WEIGHT
    //}
    int numNodes = json_array_size(nodes);
    s << "set numNodes " << numNodes << " \n";
    s << "set rho " << 2383.0 * 3.61273e-5 / 1000 << " \n"; //kilo-lb per cubic inch
    s << "set g -1.0 \n";                             // lb force

    s << "set volumeOfWall " << volumeOfWall << "\n"; // TODO

    s << "pattern Plain 100 \"Constant\" { \n";
    for (int i = 0; i < json_array_size(nodes); i++)
    {
      int nodeTag = json_integer_value(json_object_get(json_array_get(nodes, i), "name"));
      s << "load " << nodeTag << " 0.0 [expr ($volumeOfWall*$rho*$g)/$numNodes];#SELF-WEIGHT\n";
    }

    s << "} \n";

    if (analysisType == 1) // cyclic
    {

      // apply gravity

      s << "constraints Plain\n";
      s << "system UmfPack\n";
      s << "test NormDispIncr 1.0e-3 30\n";
      s << "algorithm ModifiedNewton\n";
      s << "numberer RCM\n";
      s << "integrator LoadControl 1\n";
      s << "analysis Static\n";
      s << "analyze " << 1 << "\n\n";

      s << "puts \"gravity is done.\""
        << "\n\n";

      s << myRecorderStr.str(); // add recorder
      s << "eval \"recorder Node -file disp.out -nodeRange 1 " +std::to_string(numNodes)+ " -dof 1 2 disp\" \n";
      s << "eval \"recorder Node -file dispx.out -nodeRange 1 " +std::to_string(numNodes)+ " -dof 1 disp\" \n";
      s << "eval \"recorder Node -file dispy.out -nodeRange 1 " +std::to_string(numNodes)+ " -dof 2 disp\" \n";

      s << "\nsystem UmfPack \n";
      s << "constraints Penalty 1.0e10 1.0e10 \n";
      s << "test NormDispIncr 1.0e-4 50 0 \n";
      s << "algorithm Newton \n";
      s << "numberer RCM \n";
      s << "integrator LoadControl 1. \n";
      s << "analysis Static \n";

      //s << "set nPts "<< nPts <<" \n";

      s << "for {set i 0} {$i < [expr $nPts]} {incr i 1} { \n";
      s << "set ok [analyze 1] \n";
      s << "if {$ok != 0} { \n";
      s << "test NormDispIncr 1.0e-3 1000 0 \n";
      s << "algorithm Newton -initial \n";
      s << "set ok [analyze 1] \n";
      s << "test NormDispIncr 1.0e-4 500 0 \n";
      s << "algorithm Newton \n";
      s << "} \n";
      s << "if {$ok != 0} {	 \n";
      s << "set i [expr $nPts]; \n";
      s << "} \n";
      s << "set progress [expr ($i*1.0)/($nPts*1.0) * 100.]\n";
      s << "puts \"$progress%\"\n";
      s << "} \n\n";

      //s << "after 1000\n";
      s << "puts \"cyclic is done.\""
        << "\n\n";

      s << "remove recorders \n";
      s << "wipe  \n";
    }
  }

  return 0;
}

// seperate for multi events
int OpenSeesTclBuilder::processEvent(ofstream &s,
                                       json_t *event,
                                       int &numPattern,
                                       int &numSeries)
{
  json_t *timeSeries;
  json_t *pattern;

  const char *eventType = json_string_value(json_object_get(event, "type"));
  if (strcmp(eventType, "Seismic") == 0 || strcmp(eventType, "Cyclic") == 0) // TODO: seperate Seismic with static cyclic
  {
    analysisType = 1;
    numSteps = json_integer_value(json_object_get(event, "numSteps"));
    dT = json_number_value(json_object_get(event, "dT"));
  }
  else
    return -1;

  std::map<string, int> timeSeriesList;
  std::map<string, int>::iterator it;

  int index;
  json_t *timeSeriesArray = json_object_get(event, "timeSeries");
  json_array_foreach(timeSeriesArray, index, timeSeries)
  {
    const char *subType = json_string_value(json_object_get(timeSeries, "type"));
    if (strcmp(subType, "PathValue") == 0)
    {
      double dt = json_number_value(json_object_get(timeSeries, "dt"));
      json_t *data = json_object_get(timeSeries, "values");
      s << "timeSeries Path " << numSeries << " -dt " << dt;
      s << " -values \{ ";
      json_t *dataV;
      int dataIndex;

      json_array_foreach(data, dataIndex, dataV)
      {
        dispVec.push_back(json_number_value(json_array_get(dataV, 0)));
        s << json_number_value(json_array_get(dataV, 0)) << " ";
      }
      s << " }\n";

      int nPts = json_array_size(data);
      s << "set nPts " << nPts << " \n";

      string name(json_string_value(json_object_get(timeSeries, "name")));
      //printf("%s\n", name.c_str());
      timeSeriesList[name] = numSeries;
      numSeries++;
    }
    else if (strcmp(subType, "Constant") == 0)
    {
      double factor = json_number_value(json_object_get(timeSeries, "factor"));
      s << "timeSeries Constant " << numSeries << " "
        << "-factor " << factor << "\n";
      string name(json_string_value(json_object_get(timeSeries, "name")));
      //printf("%s\n", name.c_str());
      timeSeriesList[name] = numSeries;
      numSeries++;
    }
    else if (strcmp(subType, "Linear") == 0)
    {
      double factor = json_number_value(json_object_get(timeSeries, "factor"));
      s << "timeSeries Linear " << numSeries << " "
        << "-factor " << factor << "\n";
      string name(json_string_value(json_object_get(timeSeries, "name")));
      //printf("%s\n", name.c_str());
      timeSeriesList[name] = numSeries;
      numSeries++;
    }
  }

  json_t *boundaryConditionsArray = json_object_get(event, "boundaryConditions");
  json_t *loadsArray = json_object_get(event, "loads");
  size_t indexLoads;
  json_t *loadJson;
  json_array_foreach(loadsArray, indexLoads, loadJson)
  {
    json_array_append_new(boundaryConditionsArray, loadJson);
  }
  json_t *patternArray = json_object_get(event, "pattern");
  patternArray = boundaryConditionsArray;

  int nodeTag;
  string cline;
  string floor;
  string dof;
  double dofValue;

  json_t *info;
  int indexLoadInfo;
  double startPoint[2];
  double endPoint[2];

  int nodesOnline[10000] = {-1};

  string patternName, patternType, timeSeriesName, loadType, positionType;
  json_t *positions, *position, *dofs, *scales;
  int pointPositionIndex, linePositionIndex;

  json_array_foreach(patternArray, index, pattern)
  {
    patternName = json_string_value(json_object_get(pattern, "name"));
    //printf("pattern name is %s\n", patternName.c_str());
    patternType = json_string_value(json_object_get(pattern, "type"));
    timeSeriesName = json_string_value(json_object_get(pattern, "timeSeries"));

    if (patternType.find("Force") != string::npos)
      loadType = "Force";
    else if (patternType.find("Displacement") != string::npos)
      loadType = "Displacement";
    else
      printf("loadType is not found! \n");

    if (patternType.find("Point") != string::npos)
      positionType = "Point";
    else if (patternType.find("Line") != string::npos)
      positionType = "Line";
    else
      printf("positionType is not found! \n");

    s << "\n # " << patternName << " \n";

    int series = 0;
    //printf("%s\n", timeSeriesName.c_str());
    it = timeSeriesList.find(timeSeriesName);
    if (it != timeSeriesList.end())
      series = it->second;
    int seriesTag = timeSeriesList[timeSeriesName];

    positions = json_object_get(pattern, "positions");

    if (positionType.compare("Point") == 0)
    {

      s << "pattern Plain " << numPattern << " " << series << " {\n";
      json_array_foreach(positions, pointPositionIndex, position)
      {
        cline = json_string_value(json_object_get(position, "cline"));
        floor = json_string_value(json_object_get(position, "floor"));
        nodeTag = getNode(stoi(cline), stoi(floor));

        dofs = json_object_get(position, "dofs");
        scales = json_object_get(position, "scales");

        for (int dofIndex = 0; dofIndex < json_array_size(dofs); dofIndex++)
        {
          string thisDof(json_string_value(json_array_get(dofs, dofIndex)));
          double thisScale = (json_number_value(json_array_get(scales, dofIndex)));

          if (loadType.compare("Displacement") == 0)
          {
            s << "  sp " << nodeTag << " " << thisDof << " " << thisScale << "\n";
          }
          if (loadType.compare("Force") == 0)
          {

            if (thisDof.compare("1") == 0)
            {
              s << "  load " << nodeTag << " " << thisScale << " 0.0" << "\n";
            }
            else if (thisDof.compare("2") == 0)
            {
              s << "  load " << nodeTag << " " << "0.0" << " " << thisScale << "\n";
            }
            else if (thisDof.compare("3") == 0)
            {
              s << "  load " << nodeTag << " "
                << "0.0 0.0"
                << " " << thisScale << "\n";
            }
          }
        }
      }
      s << "}\n";
    }

    if (positionType.compare("Line") == 0)
    {
      s << "pattern Plain " << numPattern << " " << series << " {\n";
      json_array_foreach(positions, linePositionIndex, position)
      {
        json_t *startPointJson = json_object_get(position, "startPoint");
        json_t *endPointJson = json_object_get(position, "endPoint");
        int startPointCline = atoi(json_string_value(json_object_get(startPointJson, "cline")));
        int startPointFloor = atoi(json_string_value(json_object_get(startPointJson, "floor")));
        int endPointCline = atoi(json_string_value(json_object_get(endPointJson, "cline")));
        int endPointFloor = atoi(json_string_value(json_object_get(endPointJson, "floor")));

        json_t *infoDataArray = json_object_get(position, "data");
        size_t indexInfoData;
        json_t *infoData;

        dofs = json_object_get(position, "dofs");
        scales = json_object_get(position, "scales");

        int startNodeTag = getNode(startPointCline, startPointFloor);
        int endNodeTag = getNode(endPointCline, endPointFloor);

        int findStartPoint = getNodeCrdByTag(startNodeTag, startPoint);
        int findEndPoint = getNodeCrdByTag(endNodeTag, endPoint);

        if (findStartPoint == 1)
        {
          //printf("startPoint tag %d \n", startNodeTag);
          //printf("startPoint x %f \n", startPoint[0]);
          //printf("startPoint y %f \n", startPoint[1]);
        }
        if (findEndPoint == 1)
        {
          //printf("endPoint tag %d \n", endNodeTag);
          //printf("endPoint x %f \n", endPoint[0]);
          //printf("endPoint y %f \n", endPoint[1]);
        }

        int numNodesOnLine = findNodesOnStraigntLine(startPoint, endPoint, nodesOnline);

        if (numNodesOnLine > 0)
        {
          //printf("Nodes on line, 1st: %d \n", nodesOnline[0]);

          for (int nodeIndex = 0; nodeIndex < 10000; nodeIndex++)
          {
            if (nodesOnline[nodeIndex] > 0)
            {
              nodeTag = nodesOnline[nodeIndex];

              for (int dofIndex = 0; dofIndex < json_array_size(dofs); dofIndex++)
              {
                string loadDof = json_string_value(json_array_get(dofs, dofIndex));
                double loadValue = json_number_value(json_array_get(scales, dofIndex));
                if (loadType.compare("Displacement") == 0)
                {
                  s << "  sp " << nodeTag << " " << loadDof << " " << loadValue << "\n";
                }
                if (loadType.compare("Force") == 0)
                {
                  if (loadDof.compare("1") == 0)
                  {
                    s << "  load " << nodeTag << " " << loadValue / numNodesOnLine << " 0.0"
                      << "\n";
                  }
                  else if (loadDof.compare("2") == 0)
                  {
                    s << "  load " << nodeTag << " "
                      << "0.0"
                      << " " << loadValue / numNodesOnLine << "\n";
                  }
                  else if (loadDof.compare("3") == 0)
                  {
                    s << "  load " << nodeTag << " "
                      << "0.0 0.0"
                      << " " << loadValue / numNodesOnLine << "\n";
                  }
                }
              }
            }
            else
            {
              break;
            }
          }
        }
      }
      s << "}\n";
    }

    numPattern++;
  }

  //  printf("%d %d %f\n",analysisType, numSteps, dT);

  return 0;
}

int OpenSeesTclBuilder::getNode(int cline, int floor)
{

  int numMapObjects = json_array_size(mapping);
  for (int i = 0; i < numMapObjects; i++)
  {
    json_t *mapObject = json_array_get(mapping, i);
    int c = json_integer_value(json_object_get(mapObject, "cline"));
    if (c == cline)
    {
      int f = json_integer_value(json_object_get(mapObject, "floor"));
      if (f == floor)
        return json_integer_value(json_object_get(mapObject, "node"));
    }
  }
  return -1;
}

int OpenSeesTclBuilder::findNodesOnStraigntLine(double pt1[], double pt2[], int nodesOnline[])
{
  //printf("Try to find nodes defined by (x1=%f y1=%f) and (x2=%f y2=%f)\n", pt1[0], pt1[1], pt2[0], pt2[1]);

  double x1 = pt1[0];
  double y1 = pt1[1];
  double x2 = pt2[0];
  double y2 = pt2[1];

  int nodeName;

  int index, count;
  //int nodesOnline[10000]={-1};
  float x, y;
  json_t *node;
  json_t *crd;
  count = 0;
  json_array_foreach(nodes, index, node)
  {
    crd = (json_object_get(node, "crd"));
    x = json_number_value(json_array_get(crd, 0));
    y = json_number_value(json_array_get(crd, 1));

    float dAB = sqrt(pow(x1 - x2, 2.0) + pow(y1 - y2, 2.0)); // distance between pt1 and pt2
    float dAC = sqrt(pow(x1 - x, 2.0) + pow(y1 - y, 2.0));   // distance between pt1 and pt2
    float dBC = sqrt(pow(x2 - x, 2.0) + pow(y2 - y, 2.0));   // distance between pt1 and pt2

    if (abs(dAC + dBC - dAB) < 0.001) // the tolarence could be smaller
    {
      nodeName = json_integer_value(json_object_get(node, "name"));
      //printf("Node found: %d (x=%f y=%f) \n", nodeName, x, y);
      nodesOnline[count] = nodeName;
      count++;
    }
  }
  return count;
}

int OpenSeesTclBuilder::getNodeCrdByTag(int nodeTag, double pt[])
{
  json_t *node;
  json_t *crd;
  int index;
  int nodeName;

  json_array_foreach(nodes, index, node)
  {
    nodeName = json_integer_value(json_object_get(node, "name"));
    if (nodeName == nodeTag)
    {
      crd = (json_object_get(node, "crd"));
      pt[0] = json_number_value(json_array_get(crd, 0));
      pt[1] = json_number_value(json_array_get(crd, 1));
      return 1;
    }
  }

  return -1;
}
