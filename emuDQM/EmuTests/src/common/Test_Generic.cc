#include "Test_Generic.h"

using namespace XERCES_CPP_NAMESPACE;

int Test_Generic::getNumStrips(std::string cscID)
{
	if ((cscID.find("ME+1.3") == 0) || (cscID.find("ME-1.3") ==0 )) return 64;
	else return 80;
}

Test_Generic::Test_Generic(std::string dfile): dataFile(dfile) {
  init();
}

Test_Generic::~Test_Generic() 
{
  for (cscTestCanvases::iterator itr=tcnvs.begin(); itr != tcnvs.end(); ++itr) {
    TestCanvases& cnvs = itr->second;
    TestCanvases::iterator c_itr;
    for (c_itr=cnvs.begin(); c_itr != cnvs.end(); ++c_itr) {
      delete c_itr->second;
    }
  }
	
}

void Test_Generic::init() {
  nTotalEvents = 0;
  nBadEvents = 0;
  imgW = DEF_WIDTH;
  imgH = DEF_HEIGHT;
  bin_checker.output1().hide();
  bin_checker.output2().hide();
  bin_checker.crcALCT(true);
  bin_checker.crcTMB (true);
  bin_checker.crcCFEB(true);
  bin_checker.modeDDU(true);
}

int Test_Generic::loadTestCfg() 
{
  if (configFile == "") {
    return 1;
  }

  XMLPlatformUtils::Initialize();
  XercesDOMParser *parser = new XercesDOMParser();
  parser->setValidationScheme(XercesDOMParser::Val_Always);
  parser->setDoNamespaces(true);
  parser->setDoSchema(true);
  parser->setValidationSchemaFullChecking(false); // this is default
  parser->setCreateEntityReferenceNodes(true);  // this is default
  parser->setIncludeIgnorableWhitespace (false);

  parser->parse(configFile.c_str());
  DOMDocument *doc = parser->getDocument();
  DOMNodeList *l = doc->getElementsByTagName( XMLString::transcode("Booking") );
  if( l->getLength() != 1 ){
    //  LOG4CPLUS_ERROR (logger_, "There is not exactly one Booking node in configuration");
    return 1;
  }
  DOMNodeList *itemList = doc->getElementsByTagName( XMLString::transcode("TestResult") );
  if( itemList->getLength() == 0 ){
    //   LOG4CPLUS_ERROR (logger_, "There no histograms to book");
    return 1;
  }
  for(uint32_t i=0; i<itemList->getLength(); i++){
    std::map<std::string, std::string> obj_info;
    std::map<std::string, std::string>::iterator itr;
    DOMNodeList *children = itemList->item(i)->getChildNodes();
    for(unsigned int i=0; i<children->getLength(); i++){
      std::string paramname = std::string(XMLString::transcode(children->item(i)->getNodeName()));
      if ( children->item(i)->hasChildNodes() ) {
	std::string param = std::string(XMLString::transcode(children->item(i)->getFirstChild()->getNodeValue()));
	obj_info[paramname] = param;
      }
    }
   
    itr = obj_info.find("Test"); 
    if ((itr != obj_info.end()) && (itr->second == testID)) {
	itr = obj_info.find("Name");
      if ((itr != obj_info.end()) && (itr->second != "")) {
	std::cout << "Found info for " << itr->second << std::endl;
	xmlCfg[itr->second] = obj_info;
      }
    }

    /*    if (obj_info.size() > 0) {
      params.clear();
      for (itr = obj_info.begin(); itr != obj_info.end(); ++itr) {
	params[itr->first] = itr->second;
      }
    }
    */
  }
  delete parser;
  return 0;
}

TestData2D parseMask(std::string s)
{

  TestData2D mask;
  mask.Nbins = 80;
  mask.Nlayers = 6;	
  memset(mask.content, 0, sizeof (mask.content));
  memset(mask.cnts, 0, sizeof (mask.cnts));
  
  std::string tmp = s;
  std::string::size_type start_pos = tmp.find("(");
  std::string::size_type end_pos = tmp.find(")");
  char* stopstring = NULL;

  while ((end_pos != std::string::npos) && (start_pos != std::string::npos))
    {
      std::string range_pair = tmp.substr(start_pos+1, end_pos);
      tmp.replace(start_pos,end_pos+1,"");
      if (range_pair.find(":") != std::string::npos) {
        int layer = strtol(range_pair.substr(0,range_pair.find(":")).c_str(),  &stopstring, 10);
        std::string chans = range_pair.substr(range_pair.find(":")+1, range_pair.length());
	
        if (chans.find("-") != std::string::npos) {
	  int ch_start=0;
	  int ch_end=0;
	  ch_start = strtol(chans.substr(0,chans.find("-")).c_str(),  &stopstring, 10);
	  ch_end = strtol(chans.substr(chans.find("-")+1,chans.length()).c_str(),  &stopstring, 10);
          for (int i=ch_start; i<= ch_end; i++) {
	    mask.content[layer-1][i-1]=1;
	    std::cout << Form("mask chan %d:%d", layer, i) << std::endl; 

	  }
	} else {
	  int chan = strtol(chans.c_str(),  &stopstring, 10);
	  mask.content[layer-1][chan-1] = 1;
	  std::cout << Form("mask chan %d:%d", layer, chan) << std::endl;
	}
      }
      end_pos = tmp.find(")");
      start_pos = tmp.find("(");
      
    }
  return mask;
}


int Test_Generic::loadMasks() 
{
  if (masksFile == "") {
    return 1;
  }

  XMLPlatformUtils::Initialize();
  XercesDOMParser *parser = new XercesDOMParser();
  parser->setValidationScheme(XercesDOMParser::Val_Always);
  parser->setDoNamespaces(true);
  parser->setDoSchema(true);
  parser->setValidationSchemaFullChecking(false); // this is default
  parser->setCreateEntityReferenceNodes(true);  // this is default
  parser->setIncludeIgnorableWhitespace (false);

  parser->parse(masksFile.c_str());
  DOMDocument *doc = parser->getDocument();
  DOMNodeList *l = doc->getElementsByTagName( XMLString::transcode("Masks") );
  if( l->getLength() != 1 ){
    //  LOG4CPLUS_ERROR (logger_, "There is not exactly one Booking node in configuration");
    return 1;
  }
  DOMNodeList *itemList = doc->getElementsByTagName( XMLString::transcode("CSCMasks") );
  if( itemList->getLength() == 0 ){
    //   LOG4CPLUS_ERROR (logger_, "There no histograms to book");
    return 1;
  }
  for(uint32_t i=0; i<itemList->getLength(); i++){
    std::map<std::string, std::string> obj_info;
    std::map<std::string, std::string>::iterator itr;
    DOMNodeList *children = itemList->item(i)->getChildNodes();
    for(unsigned int i=0; i<children->getLength(); i++){
      std::string paramname = std::string(XMLString::transcode(children->item(i)->getNodeName()));
      if ( children->item(i)->hasChildNodes() ) {
	std::string param = std::string(XMLString::transcode(children->item(i)->getFirstChild()->getNodeValue()));
	obj_info[paramname] = param;
      }
    }
    itr = obj_info.find("CSC");
    if (itr != obj_info.end()) {
      std::cout << "Found masks for " << itr->second << std::endl;
      if (obj_info["CFEBChans"] != "") {
	tmasks[itr->second]=parseMask(obj_info["CFEBChans"]);
      }
    }
  }
  delete parser;
  return 0;
}


void Test_Generic::setCSCMapFile(std::string filename)
{
  if (filename != "") {
    cscMapFile = filename;
    cscMapping  = CSCReadoutMappingFromFile(cscMapFile);
  }


}

std::string Test_Generic::getCSCTypeLabel(int endcap, int station, int ring )
{
  std::string label = "Unknown";
  std::ostringstream st;
  if ((endcap > 0) && (station>0) && (ring>0)) {
    if (endcap==1) {
      st << "ME+" << station << "." << ring;
      label = st.str();
    } else if (endcap==2) {
      st << "ME-" << station << "." << ring;
      label = st.str();
    } else {
      label = "Unknown";
    }
  }
  return label;
}


std::string Test_Generic::getCSCFromMap(int crate, int slot)
{
  int iendcap = -1;
  int istation = -1;
  int iring = -1;
  int cscposition = -1;

  int id = cscMapping.chamber(iendcap, istation, crate, slot, -1);
  if (id==0) {
    return "";
  }
  CSCDetId cid( id );
  iendcap = cid.endcap();
  istation = cid.station();
  iring = cid.ring();
  cscposition = cid.chamber();

  std::string tlabel = getCSCTypeLabel(iendcap, istation, iring );
  return tlabel+"."+Form("%02d", cscposition);
}


std::map<int, std::string> ParseAxisLabels(std::string s)
{

  std::map<int, std::string> labels;
  std::string tmp = s;
  std::string::size_type pos = tmp.find("|");
  char* stopstring = NULL;

  while (pos != std::string::npos)
    {
      std::string label_pair = tmp.substr(0, pos);
      tmp.replace(0,pos+1,"");
      if (label_pair.find("=") != std::string::npos) {
        int nbin = strtol(label_pair.substr(0,label_pair.find("=")).c_str(),  &stopstring, 10);
        std::string label = label_pair.substr(label_pair.find("=")+1, label_pair.length());
        while (label.find("\'") != std::string::npos) {
          label.erase(label.find("\'"),1);
        }
        labels[nbin] = label;
      }
      pos = tmp.find("|");
    }
  return labels;
}


int applyParameters(TH1* object, bookParams& params)
{
  char* stopstring;
  std::map<std::string, std::string>::iterator itr;
  // !!! TODO: Add object class check
  if (object != NULL) {
    // std::cout << "Booked " << getFullName() << std::endl;
    if (((itr = params.find("XTitle")) != params.end()) ||
        ((itr = params.find("XLabel")) != params.end())) {
      object->SetXTitle(itr->second.c_str());
    }
    if (((itr = params.find("YTitle")) != params.end()) ||
        ((itr = params.find("YLabel")) != params.end())) {
      object->SetYTitle(itr->second.c_str());
    }
    if (((itr = params.find("ZTitle")) != params.end()) ||
        ((itr = params.find("ZLabel")) != params.end())) {
      object->SetZTitle(itr->second.c_str());
    }

    if ((itr = params.find("SetOption")) != params.end()) {
      object->SetOption(itr->second.c_str());
      //object->Draw();
    }

    /*
      if ((itr = params.find("SetOptStat")) != params.end()) {
      gStyle->SetOptStat(itr->second.c_str());

      }
    */

    if ((itr = params.find("SetStats")) != params.end()) {
      int stats = strtol( itr->second.c_str(), &stopstring, 10 );
      object->SetStats(bool(stats));
    }


   object->SetFillColor(48);

    if ((itr = params.find("SetFillColor")) != params.end()) {
      int color = strtol( itr->second.c_str(), &stopstring, 10 );
      object->SetFillColor(color);
    }

    if ((itr = params.find("SetXLabels")) != params.end()) {
      std::map<int, std::string> labels = ParseAxisLabels(itr->second);
      for (std::map<int, std::string>::iterator l_itr = labels.begin(); l_itr != labels.end(); ++l_itr)
        {
          object->GetXaxis()->SetBinLabel(l_itr->first, l_itr->second.c_str());
        }


    }

    if ((itr = params.find("SetYLabels")) != params.end()) {
      std::map<int, std::string> labels = ParseAxisLabels(itr->second);
      for (std::map<int, std::string>::iterator l_itr = labels.begin(); l_itr != labels.end(); ++l_itr)
        {
          object->GetYaxis()->SetBinLabel(l_itr->first, l_itr->second.c_str());
        }
    }
    if ((itr = params.find("LabelsOption")) != params.end()) {
      std::string st = itr->second;
      if (st.find(",") != std::string::npos) {
        std::string opt = st.substr(0,st.find(",")) ;
        std::string axis = st.substr(st.find(",")+1,st.length());
        object->LabelsOption(opt.c_str(),axis.c_str());
      }
    }


    if ((itr = params.find("SetLabelSize")) != params.end()) {
      std::string st = itr->second;
      if (st.find(",") != std::string::npos) {
        double opt = atof(st.substr(0,st.find(",")).c_str()) ;
        std::string axis = st.substr(st.find(",")+1,st.length());
        object->SetLabelSize(opt,axis.c_str());
      }
    }
    if ((itr = params.find("SetTitleOffset")) != params.end()) {
      std::string st = itr->second;
      if (st.find(",") != std::string::npos) {
        double opt = atof(st.substr(0,st.find(",")).c_str()) ;
        std::string axis = st.substr(st.find(",")+1,st.length());
        object->SetTitleOffset(opt,axis.c_str());
      }
    }
    if ((itr = params.find("SetMinimum")) != params.end()) {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      object->SetMinimum(opt);
    }
    if ((itr = params.find("SetMaximum")) != params.end()) {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      object->SetMaximum(opt);
    }

    if ((itr = params.find("SetNdivisionsX")) != params.end()) {
      int opt = strtol( itr->second.c_str(), &stopstring, 10 );
      if (object) {
        object->SetNdivisions(opt,"X");
        object->GetXaxis()->CenterLabels(true);
      }

    }

    if ((itr = params.find("SetNdivisionsY")) != params.end()) {
      int opt = strtol( itr->second.c_str(), &stopstring, 10 );
      if (object) {
        object->SetNdivisions(opt,"Y");
        object->GetYaxis()->CenterLabels(true);
      }
    }
    if ((itr = params.find("SetTickLengthX")) != params.end()) {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object) {
        object->SetTickLength(opt,"X");
      }
    }

    if ((itr = params.find("SetTickLengthY")) != params.end()) {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object) {
        object->SetTickLength(opt,"Y");
      }
    }

    if ((itr = params.find("SetLabelSizeX")) != params.end()) {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object) {
        object->SetLabelSize(opt,"X");

      }
    }

    if ((itr = params.find("SetLabelSizeY")) != params.end()) {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object) {
	//        object->GetYaxis()->SetLabelSize(opt);
        object->SetLabelSize(opt,"Y");
      }
    }
    if ((itr = params.find("SetErrorOption")) != params.end()) {
      std::string st = itr->second;
      if (object) {
        reinterpret_cast<TProfile*>(object)->SetErrorOption(st.c_str());
      }
    }


    if ((itr = params.find("SetLabelSizeZ")) != params.end()) {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object) {
	//        object->GetZaxis()->SetLabelSize(opt);
        object->SetLabelSize(opt,"Z");
	/*
	  TPaletteAxis *palette = (TPaletteAxis*)object->GetListOfFunctions()->FindObject("palette");
	  if (palette != NULL) {
	  palette->SetLabelSize(opt);
	  }
	*/
      }
    }

  }
  return 0;
}


void Test_Generic::bookTestsForCSC(std::string cscID) {
  MonHistos cschistos;
  TestCanvases csccnvs;
  char *stopstring;
  for (testParamsCfg::iterator itr=xmlCfg.begin(); itr != xmlCfg.end(); ++itr) {
    bookParams& params = itr->second;
    if (params.find("Type") != params.end()) {
      std::string cnvtype = params["Type"];
	std::string name = cscID+"_"+testID+"_"+params["Name"];
	std::string title = cscID+": "+testID+" "+params["Title"];
	double xmin=0., xmax=0.; int xbins=0;
	double ymin=0., ymax=0.; int ybins=0;
	std::string xtitle = params["XTitle"];
	std::string ytitle = params["YTitle"];
	double low0limit=0., low1limit=0.;
	double high0limit=0., high1limit=0.;
	if (params["XMin"] != "") {
	  xmin = atof(params["XMin"].c_str());
	}
	if (params["XMax"] != "") {
	  xmax = atof(params["XMax"].c_str());
	}
	if (params["YMin"] != "") {
	  ymin = atof(params["YMin"].c_str());
	}
	if (params["YMax"] != "") {
	  ymax = atof(params["YMax"].c_str());
	}
	if (params["XBins"] != "") {
	  xbins = strtol(params["XBins"].c_str(), &stopstring, 10);
	}
	if (params["YBins"] != "") {
	  ybins = strtol(params["YBins"].c_str(), &stopstring, 10);
	}
      if (cnvtype == "cfeb_cnv") {
	// = Set actual number of strips depending on Chamber type
	xbins = getNumStrips(cscID);
	xmax = getNumStrips(cscID);

	if (params["Low0Limit"] != "") {
	  low0limit = atof(params["Low0Limit"].c_str());
	}
	if (params["Low1Limit"] != "") {
	  low1limit = atof(params["Low1Limit"].c_str());
	}
	if (params["High0Limit"] != "") {
	  high0limit = atof(params["High0Limit"].c_str());
	}
	if (params["High1Limit"] != "") {
	  high1limit = atof(params["High1Limit"].c_str());
	}

	// TestCanvas_6gr1h* cnv = new TestCanvas_6gr1h((cscID+"_CFEB02_R03").c_str(), (cscID+": CFEB02 R03").c_str(),80, 0.0, 80.0, 60, 0., 6.0);	
	TestCanvas_6gr1h* cnv = new TestCanvas_6gr1h(name, title,xbins, xmin, xmax, ybins, ymin, ymax);
	cnv->SetXTitle(xtitle);
	cnv->SetYTitle(ytitle);
	cnv->AddTextTest(testID);
	cnv->AddTextResult(params["Title"]);
	cnv->SetLimits(low1limit,low0limit, high0limit, high1limit);
	csccnvs[itr->first]=cnv;
      }

      if (cnvtype.find("h") == 0) {
	if (cnvtype.find("h1") != std::string::npos) {
	  cschistos[itr->first] = new TH1F((cnvtype+"_"+name).c_str(), title.c_str(), xbins, xmin, xmax);
	} else
	  if (cnvtype.find("h2") != std::string::npos) {
	    cschistos[itr->first] = new TH2F((cnvtype+"_"+name).c_str(), title.c_str(), xbins, xmin, xmax, ybins, ymin, ymax);
	  } else
	    if (cnvtype.find("hp") != std::string::npos) {
	      cschistos[itr->first] = new TProfile((cnvtype+"_"+name).c_str(), title.c_str(), xbins, xmin, xmax);
	    }
	applyParameters(cschistos[itr->first], params);
      }


    } 
  }

  tcnvs[cscID] = csccnvs;

  hFormatErrors[cscID] = new TH2F((cscID+"_"+testID+"_FormatErrors").c_str(), "CSC Format Errors", 1, 0, 1, 20, 0, 20);
  hFormatErrors[cscID]->SetOption("textcolz");

  mhistos[cscID]=cschistos;

  //  return cschistos;
}


void Test_Generic::analyze(const char * data, int32_t dataSize, uint32_t errorStat, int32_t nodeNumber) {
  nTotalEvents++;

  //   uint32_t BinaryErrorStatus = 0, BinaryWarningStatus = 0;
  const uint16_t *tmp = reinterpret_cast<const uint16_t *>(data);
  if( bin_checker.check(tmp,dataSize/sizeof(short)) < 0 ){
    //   No ddu trailer found - force checker to summarize errors by adding artificial trailer
    const uint16_t dduTrailer[4] = { 0x8000, 0x8000, 0xFFFF, 0x8000 };
    tmp = dduTrailer; bin_checker.check(tmp,uint32_t(4));
  }

  if(bin_checker.errors() != 0) {
    std::cout << "Evt#" << std::dec << nTotalEvents << ": Nonzero Binary Errors Status is observed: 0x"<< std::hex << bin_checker.errors() << std::endl;
    doBinCheck();
    return;
  }

  CSCDDUEventData dduData((uint16_t *) data);

  std::vector<CSCEventData> chamberDatas;
  chamberDatas = dduData.cscData();

  for(std::vector<CSCEventData>::iterator chamberDataItr = chamberDatas.begin(); 
      chamberDataItr != chamberDatas.end(); ++chamberDataItr) {
    analyzeCSC(*chamberDataItr);
  }
	
}


void Test_Generic::doBinCheck() {

  std::map<int,long> checkerErrors = bin_checker.errorsDetailed();
  std::map<int,long>::const_iterator chamber = checkerErrors.begin();
  while( chamber != checkerErrors.end() ){
    // int ChamberID     = chamber->first;
    int CrateID = (chamber->first>>4) & 0xFF;
    int DMBSlot = chamber->first & 0xF;
    if ((CrateID ==255) ||
        (chamber->second & 0x80)) { chamber++; continue;} // = Skip chamber detection if DMB header is missing (Error code 6)

    std::string cscID = getCSCFromMap(CrateID, DMBSlot);
    if (cscID == "") continue;
  
    cscTestData::iterator td_itr = tdata.find(cscID);
    if ( (td_itr == tdata.end()) || (tdata.size() == 0) ) {
      std::cout << "Found " << cscID << std::endl;
      initCSC(cscID);
      //      mhistos[cscID] = bookMonHistosCSC(cscID);
    }
    
    bool isCSCError = false;
    if (hFormatErrors[cscID]) {
       for(int bit=5; bit<24; bit++)
        if( chamber->second & (1<<bit) ) {
          isCSCError = true;
          hFormatErrors[cscID]->Fill(0.,bit-5);
	}
    }
    nCSCBadEvents[cscID]++;
    chamber++;
  }
}


void Test_Generic::finish() {

  char* stopstring;
  struct tm* clock;
  struct stat attrib;
  stat(dataFile.c_str(), &attrib);
  clock = localtime(&(attrib.st_mtime));
  std::string dataTime=asctime(clock);
  time_t now = time(NULL);  
  clock = localtime(&now);
  std::string testTime=asctime(clock);

  std::string rpath = "Test_"+testID+"/"+outDir;
  TString command = Form("mkdir -p %s", rpath.c_str());
  gSystem->Exec(command.Data());
  std::ofstream fres((rpath+"/test_results.js").c_str());
  int res=0;
  int sum_res=res;

  gStyle->SetPalette(1,0);

  std::string filename="Test_"+testID+"/"+rootFile;
  TFile root_res(filename.c_str(), "recreate");
  if (!root_res.IsZombie()) {
    root_res.cd();
    TDirectory* f = root_res.mkdir(("Test_"+testID).c_str());

    
    for (cscTestData::iterator td_itr = tdata.begin(); td_itr != tdata.end(); ++td_itr) {
      bool fEnoughData = true;
      sum_res=0;
      std::string cscID = td_itr->first;


      TDirectory * rdir = f->mkdir((cscID).c_str());
      std::string path = rpath+"/"+cscID+"/";
      TString command = Form("mkdir -p %s", path.c_str());
      gSystem->Exec(command.Data());
      std::ofstream csc_fres((path+"/test_results.js").c_str());
      
      fres << "['"+cscID+"',[" << std::endl;
      csc_fres << "['"+cscID+"',[" << std::endl;

      rdir->cd();

      TH1F* mo = reinterpret_cast<TH1F*>(mhistos[cscID]["V00"]);
      double max_freq=0.;
      if (mo && hFormatErrors[cscID]) {
      	uint32_t nTotalEvents = nCSCEvents[cscID]+nCSCBadEvents[cscID];
      	for(int bit=1; bit<=20; bit++) {
          double freq = (100.0*hFormatErrors[cscID]->GetBinContent(1,bit))/nTotalEvents;
	  if (freq>0) mo->SetBinContent(bit, freq);
	  if (freq>max_freq) max_freq=freq;
      	}
      	mo->SetEntries(nTotalEvents);
	if (max_freq>0) mo->SetMaximum(max_freq);
	else mo->SetMaximum(1);
      }
   
      if (nCSCEvents[cscID] >= nExpectedEvents/2) {
	finishCSC(cscID);
      } else {
	std::cout << Form("%s: Not enough events for test analysis (%d events)", cscID.c_str(), nCSCEvents[cscID] ) << std::endl;
	// = Set error 
	sum_res=4;
	fres << "\t['V00','" << 4 << "']," << std::endl;
	csc_fres << "\t['V00','" << 4 << "']," << std::endl;
	fEnoughData = false;
	/*
	if (mhistos[cscID]["V00"]) {

	}	
	*/
      }



      TestData& cscdata= td_itr->second;
      TestData2D& mask = cscdata["_MASK"];

      TestCanvases& cnvs = tcnvs[cscID];
      TestCanvases::iterator c_itr;
      for (c_itr=cnvs.begin(); c_itr != cnvs.end(); ++c_itr) {
	std::string subtestID = c_itr->first;
	TestCanvas_6gr1h* cnv = dynamic_cast<TestCanvas_6gr1h*>(c_itr->second);
	TestData::iterator itr = cscdata.find(subtestID);
	if (itr != cscdata.end()) {
	  TestData2D& data = itr->second;
	  cnv->AddTextDatafile(dataFile);
	  cnv->AddTextRun(dataTime);
	  cnv->AddTextAnalysis(testTime +", version " + ANALYSIS_VER);
	  if (fEnoughData) {
	    res=cnv->Fill(data,mask);
	    if (res>sum_res) sum_res=res;
	    fres << "\t['" << itr->first << "','" << res << "']," << std::endl;
	    csc_fres << "\t['" << itr->first << "','" << res << "']," << std::endl;
	  }
	  cnv->SetCanvasSize(imgW, imgH);
	  cnv->Draw();
	  cnv->SetCanvasSize(imgW, imgH);
	  cnv->SaveAs(path+cscID+"_"+testID+"_"+subtestID+".png");
	  cnv->Write();		
	}
      }      

      MonHistos& monhistos = mhistos[cscID];
      MonHistos::iterator m_itr;
      for (m_itr=monhistos.begin(); m_itr!=monhistos.end(); ++m_itr) {
	std::string subtestID = m_itr->first;
	bookParams& params =  xmlCfg[subtestID];
	std::string descr = params["Title"];

	TStyle defStyle(*gStyle);
/*	MonitoringCanvas* cnv= new MonitoringCanvas((cscID+"_"+testID+"_"+subtestID).c_str(), (cscID+"_"+testID+"_"+subtestID).c_str(), 
						    (cscID + " "+testID+" "+descr).c_str() ,
						    1, 1, imgW, imgH);

*/
	TestCanvas_userHisto* cnv = new TestCanvas_userHisto((cscID+"_"+testID+"_"+subtestID).c_str(), 
                                                    (cscID + " "+testID+" "+descr).c_str());	
/*
	TestCanvas_1h* cnv = new TestCanvas_1h((cscID+"_"+testID+"_"+subtestID).c_str(),
                                                    (cscID + " "+testID+" "+descr).c_str(), 70, 300.0, 1000.0);
*/
	cnv->SetCanvasSize(imgW, imgH);
	cnv->AddTextTest(testID);
        cnv->AddTextResult(params["Title"]);
	cnv->AddTextDatafile(dataFile);
        cnv->AddTextRun(dataTime);
        cnv->AddTextAnalysis(testTime +", version " + ANALYSIS_VER);
	cnv->AddTextEvents(Form("%d",nCSCEvents[cscID]));
	cnv->AddTextBadEvents(Form("%d",nCSCBadEvents[cscID]));

	// cnv->cd(1);
	TVirtualPad* cPad = cnv->GetUserPad();	

	std::string leftMargin = params["SetLeftMargin"];
	if (leftMargin != "" ) {
	  cPad->SetLeftMargin(atof(leftMargin.c_str()));
	}
	std::string rightMargin = params["SetRightMargin"];
	if (rightMargin != "" ) {
	  cPad->SetRightMargin(atof(rightMargin.c_str()));
	}

	std::string logx = params["SetLogx"];
	if (logx!= "") {
	  cPad->SetLogx();
	}
	std::string logy = params["SetLogy"];
	if (logy!= "" && (m_itr->second->GetMaximum()>0.)) {
       	  cPad->SetLogy();
	}

	std::string logz = params["SetLogz"];
	if (logz!= "" && (m_itr->second->GetMaximum()>0.) ) {
       	  cPad->SetLogz();
	}

	std::string gridx = params["SetGridx"];
	if (gridx!= "" ) {
	  cPad->SetGridx();
	}

	std::string gridy = params["SetGridy"];
	if (gridy!= "" ) {
	  cPad->SetGridy();
	}

	if (params["SetStats"] != "") {
	  int stats = strtol( params["SetStats"].c_str(), &stopstring, 10 );
	  m_itr->second->SetStats(bool(stats));
	}

	std::string statOpt = params["SetOptStat"];
	//if (statOpt != "" ) {
            gStyle->SetOptStat(statOpt.c_str());
	    // } 


        gStyle->SetPalette(1,0);
//	m_itr->second->Draw();

	cnv->SetHistoObject(m_itr->second);
//	cnv->SetResultCode(1);
	cnv->Draw();
	cnv->SaveAs((path+cscID+"_"+testID+"_"+subtestID+".png").c_str());
//	cnv->Print((path+cscID+"_"+testID+"_"+subtestID+".png").c_str());
	m_itr->second->Write();
	cnv->Write();
	delete cnv;
	defStyle.cd();
      }


      f->cd();
      fres << "\t['SUMMARY','" << sum_res << "']" << std::endl;
      fres << "]],"<< std::endl;
      csc_fres << "\t['SUMMARY','" << sum_res << "']" << std::endl;
      csc_fres << "]],"<< std::endl;

    }
    root_res.Close();
  }
  saveCSCList();

}



void Test_Generic::saveCSCList()
{
  //   struct tm* clock;
  struct stat attrib;
  stat(dataFile.c_str(), &attrib);
  // clock = localtime(&(attrib.st_mtime));
  std::string dataTime=ctime(&(attrib.st_mtime));//asctime(clock);
  dataTime = dataTime.substr(0,dataTime.find("\n",0));

  std::string path = "Test_"+testID+"/"+outDir;

  std::ofstream csc_list((path+"/csc_list.js").c_str());
  csc_list << "var CSC_LIST=[\n\t['"<< dataFile << "','"<< dataTime << "',"<< std::endl;
  for (cscTestData::iterator td_itr = tdata.begin(); td_itr != tdata.end(); ++td_itr) {
    std::string cscID = td_itr->first;
    csc_list << "'"<< cscID << "'," << std::endl;
  }
  csc_list<< "]]" << std::endl;
}
