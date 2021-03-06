#include "emu/dqm/common/EmuMonitoringObject.h"

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
    if (label_pair.find("=") != std::string::npos)
    {
      int nbin = strtol(label_pair.substr(0,label_pair.find("=")).c_str(),  &stopstring, 10);
      std::string label = label_pair.substr(label_pair.find("=")+1, label_pair.length());
      while (label.find("\'") != std::string::npos)
      {
        label.erase(label.find("\'"),1);
      }
      labels[nbin] = label;
    }
    pos = tmp.find("|");
  }
  return labels;
}

EmuMonitoringObject::EmuMonitoringObject(const EmuMonitoringObject& mo)
{
  object = reinterpret_cast<MonitorElement*>(mo.object->Clone());
  type = mo.type;
  prefix = mo.prefix;
  name = mo.name;
  title = mo.title;
  params = mo.params;
}

EmuMonitoringObject& EmuMonitoringObject::operator=(const EmuMonitoringObject& mo)
{
  object = reinterpret_cast<MonitorElement*>(mo.object->Clone());
  type = mo.type;
  prefix = mo.prefix;
  name = mo.name;
  title = mo.title;
  params = mo.params;
  return *this;
}

EmuMonitoringObject::EmuMonitoringObject() :
  type(""),
  prefix(""),
  name(""),
  title("")
{
  object = NULL;
  params.clear();
}

EmuMonitoringObject::EmuMonitoringObject(DOMNode* info)
{
  object = NULL;
  parseDOMNode(info);
  Book();
}

int EmuMonitoringObject::Book()
{

  int nbinsx = 0, nbinsy = 0, nbinsz = 0;
  float xlow = 0, ylow = 0, zlow = 0;
  float xup = 0, yup = 0, zup = 0;
  char *stopstring;

  if (object != NULL)
  {
    delete object;
    object = NULL;
  }

  std::map<std::string, std::string> other_params;
  std::map<std::string, std::string>::iterator itr;
  if ((itr = params.find("XBins")) != params.end())
  {
    nbinsx = strtol( itr->second.c_str(), &stopstring, 10 );
  }
  if ((itr = params.find("YBins")) != params.end())
  {
    nbinsy = strtol( itr->second.c_str(), &stopstring, 10 );
  }
  if ((itr = params.find("ZBins")) != params.end())
  {
    nbinsz = strtol( itr->second.c_str(), &stopstring, 10 );
  }
  if ((itr = params.find("XMin")) != params.end())
  {
    xlow = strtol( itr->second.c_str(), &stopstring, 10 );
  }
  if ((itr = params.find("XMax")) != params.end())
  {
    xup = strtol( itr->second.c_str(), &stopstring, 10 );
  }
  if ((itr = params.find("YMin")) != params.end())
  {
    ylow = strtol( itr->second.c_str(), &stopstring, 10 );
  }
  if ((itr = params.find("YMax")) != params.end())
  {
    yup = strtol( itr->second.c_str(), &stopstring, 10 );
  }
  if ((itr = params.find("ZMin")) != params.end())
  {
    zlow = strtol( itr->second.c_str(), &stopstring, 10 );
  }
  if ((itr = params.find("ZMax")) != params.end())
  {
    zup = strtol( itr->second.c_str(), &stopstring, 10 );
  }


  if ((itr = params.find("XRange")) != params.end())
  {
    std::string str = itr->second;
    replace(str.begin(), str.end(), '(', ' ');
    replace(str.begin(), str.end(), ')', ' ');
    replace(str.begin(), str.end(), ':', ' ');
    std::stringstream st(str);
    st >> xlow;
    st >> xup;
  }
  if ((itr = params.find("YRange")) != params.end())
  {
    std::string str = itr->second;
    replace(str.begin(), str.end(), '(', ' ');
    replace(str.begin(), str.end(), ')', ' ');
    replace(str.begin(), str.end(), ':', ' ');
    std::stringstream st(str);
    st >> ylow;
    st >> yup;
  }
  if ((itr = params.find("ZRange")) != params.end())
  {
    std::string str = itr->second;
    replace(str.begin(), str.end(), '(', ' ');
    replace(str.begin(), str.end(), ')', ' ');
    replace(str.begin(), str.end(), ':', ' ');
    std::stringstream st(str);
    st >> zlow;
    st >> zup;
  }

  if (type.find("h1") != std::string::npos)
  {
    //  std::cout << getFullName() <<"("<< nbinsx<<","<<xlow<<","<<xup<<")"<<std::endl;
    object = (MonitorElement*) new TH1D(getFullName().c_str(), getTitle().c_str(), nbinsx, xlow, xup);
  }
  else if (type.find("h2") != std::string::npos)
  {
    // std::cout << getFullName() <<"("<< nbinsx<<","<<xlow<<","<<xup<<","<<nbinsy<<","<<ylow<<","<<yup<<")"<<std::endl;
    object = (MonitorElement*) new TH2D(getFullName().c_str(), getTitle().c_str(), nbinsx, xlow, xup, nbinsy, ylow, yup);
  }
  else if (type.find("h3") != std::string::npos)
  {
    object = (MonitorElement*) new TH3D(getFullName().c_str(), getTitle().c_str(), nbinsx, xlow, xup,
                                        nbinsy, ylow, yup, nbinsz, zlow, zup);
  }
  else if (type.find("hp2") != std::string::npos)
  {
    object = (MonitorElement*) new TProfile2D(getFullName().c_str(), getTitle().c_str(), nbinsx, xlow, xup,
             nbinsy, ylow, yup);
  }
  else if (type.find("hp") != std::string::npos)
  {
    object = (MonitorElement*) new TProfile(getFullName().c_str(), getTitle().c_str(), nbinsx, xlow, xup);
  }

  applyParameters();
  return 0;
}

int EmuMonitoringObject::applyParameters()
{
  char* stopstring;
  std::map<std::string, std::string>::iterator itr;
  // !!! TODO: Add object class check
  if (object != NULL)
  {
    // std::cout << "Booked " << getFullName() << std::endl;
    if (((itr = params.find("XTitle")) != params.end()) ||
        ((itr = params.find("XLabel")) != params.end()))
    {
      object->SetXTitle(itr->second.c_str());
    }
    if (((itr = params.find("YTitle")) != params.end()) ||
        ((itr = params.find("YLabel")) != params.end()))
    {
      object->SetYTitle(itr->second.c_str());
    }
    if (((itr = params.find("ZTitle")) != params.end()) ||
        ((itr = params.find("ZLabel")) != params.end()))
    {
      object->SetZTitle(itr->second.c_str());
    }

    if ((itr = params.find("SetOption")) != params.end())
    {
      object->SetOption(itr->second.c_str());
      //object->Draw();
    }

    /*
    if ((itr = params.find("SetOptStat")) != params.end()) {
    gStyle->SetOptStat(itr->second.c_str());

    }
    */

    if ((itr = params.find("SetStats")) != params.end())
    {
      int stats = strtol( itr->second.c_str(), &stopstring, 10 );
      object->SetStats(bool(stats));
    }


    object->SetFillColor(DEF_HISTO_COLOR);

    if ((itr = params.find("SetFillColor")) != params.end())
    {
      int color = strtol( itr->second.c_str(), &stopstring, 10 );
      object->SetFillColor(color);
    }

    if ((itr = params.find("SetXLabels")) != params.end())
    {
      std::map<int, std::string> labels = ParseAxisLabels(itr->second);
      for (std::map<int, std::string>::iterator l_itr = labels.begin(); l_itr != labels.end(); ++l_itr)
      {
        object->GetXaxis()->SetBinLabel(l_itr->first, l_itr->second.c_str());
      }


    }

    if ((itr = params.find("SetYLabels")) != params.end())
    {
      std::map<int, std::string> labels = ParseAxisLabels(itr->second);
      for (std::map<int, std::string>::iterator l_itr = labels.begin(); l_itr != labels.end(); ++l_itr)
      {
        object->GetYaxis()->SetBinLabel(l_itr->first, l_itr->second.c_str());
      }
    }
    if ((itr = params.find("LabelsOption")) != params.end())
    {
      std::string st = itr->second;
      if (st.find(",") != std::string::npos)
      {
        std::string opt = st.substr(0,st.find(",")) ;
        std::string axis = st.substr(st.find(",")+1,st.length());
        object->LabelsOption(opt.c_str(),axis.c_str());
      }
    }


    if ((itr = params.find("SetLabelSize")) != params.end())
    {
      std::string st = itr->second;
      if (st.find(",") != std::string::npos)
      {
        double opt = atof(st.substr(0,st.find(",")).c_str()) ;
        std::string axis = st.substr(st.find(",")+1,st.length());
        object->SetLabelSize(opt,axis.c_str());
      }
    }
    if ((itr = params.find("SetTitleOffset")) != params.end())
    {
      std::string st = itr->second;
      if (st.find(",") != std::string::npos)
      {
        double opt = atof(st.substr(0,st.find(",")).c_str()) ;
        std::string axis = st.substr(st.find(",")+1,st.length());
        object->SetTitleOffset(opt,axis.c_str());
      }
    }
    if ((itr = params.find("SetMinimum")) != params.end())
    {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      object->SetMinimum(opt);
    }
    if ((itr = params.find("SetMaximum")) != params.end())
    {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      object->SetMaximum(opt);
    }

    if ((itr = params.find("SetNdivisionsX")) != params.end())
    {
      int opt = strtol( itr->second.c_str(), &stopstring, 10 );
      if (object)
      {
        // object->SetNdivisions(opt,"X");
	object->SetNdivisions(object->GetNbinsX(),"X");
        object->GetXaxis()->CenterLabels(true);
      }

    }

    if ((itr = params.find("SetNdivisionsY")) != params.end())
    {
      int opt = strtol( itr->second.c_str(), &stopstring, 10 );
      if (object)
      {
        // object->SetNdivisions(opt,"Y");
	object->SetNdivisions(object->GetNbinsY(),"Y");
        object->GetYaxis()->CenterLabels(true);
      }
    }

    if ((itr = params.find("SetTickLengthX")) != params.end())
    {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object)
      {
        object->SetTickLength(opt,"X");
      }
    }

    if ((itr = params.find("SetTickLengthY")) != params.end())
    {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object)
      {
        object->SetTickLength(opt,"Y");
      }
    }

    if ((itr = params.find("SetLabelSizeX")) != params.end())
    {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object)
      {
        object->SetLabelSize(opt,"X");

      }
    }

    if ((itr = params.find("SetLabelSizeY")) != params.end())
    {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object)
      {
        //        object->GetYaxis()->SetLabelSize(opt);
        object->SetLabelSize(opt,"Y");
      }
    }
    if ((itr = params.find("SetErrorOption")) != params.end())
    {
      std::string st = itr->second;
      if (object)
      {
        reinterpret_cast<TProfile*>(object)->SetErrorOption(st.c_str());
      }
    }


    if ((itr = params.find("SetLabelSizeZ")) != params.end())
    {
      std::string st = itr->second;
      double opt = atof(st.c_str()) ;
      if (object)
      {
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


void  EmuMonitoringObject::Draw()
{
  if (object!=NULL)
  {
    object->Draw();
  }
}

int EmuMonitoringObject::Book(DOMNode* info)
{

  parseDOMNode(info);
  Book();
  return 0;
}

int EmuMonitoringObject::setObject(MonitorElement* hist)
{

  if ( hist->IsA()->InheritsFrom( "TH1" ) )
  {
    if (object != NULL)
    {
      delete object;
      object = NULL;
    }
    // object = reinterpret_cast<MonitorElement*>(hist->Clone());

    object = dynamic_cast<MonitorElement*>(hist);
    applyParameters();
  }
  return 0;
}

int EmuMonitoringObject::Add(MonitorElement* hist)
{
  if (object != NULL)
  {
    object->Add(hist);
  }
  return 0;
}


EmuMonitoringObject::~EmuMonitoringObject()
{
  if (object != NULL)
  {
    delete object;
    object = NULL;
  }
}

void EmuMonitoringObject::setName(std::string newname)
{
  name = newname;
  if (object != NULL)
  {
    object->SetName(getFullName().c_str());
  }
}

void EmuMonitoringObject::setPrefix(std::string newprefix)
{
  prefix = newprefix;
  if (object != NULL)
  {
    object->SetName(getFullName().c_str());
  }
}


void EmuMonitoringObject::setTitle(std::string newtitle)
{
  title = newtitle;
  if (object != NULL)
  {
    object->SetTitle(getTitle().c_str());
  }
}

int EmuMonitoringObject::setParameter(std::string parname, std::string parvalue)
{
  if (object != NULL)
  {
    params[parname] = parvalue;
    return 0;
  }
  else return 1;

}

void EmuMonitoringObject::SetEntries(double entries)
{
  if (object != NULL)
  {
    object->SetEntries(entries);
  }

}

void EmuMonitoringObject::SetBinContent(int nbin, double value)
{
  if (object != NULL)
  {
    object->SetBinContent(nbin, value);
  }

}

void EmuMonitoringObject::SetBinContent(int nxbin, int nybin, double value)
{
  if (object != NULL)
  {
    object->SetBinContent(nxbin,nybin,value);
  }

}


double EmuMonitoringObject::GetBinContent(int nbin)
{
  if (object != NULL)
  {
    return object->GetBinContent(nbin);
  }
  else return 0;

}

double EmuMonitoringObject::GetBinContent(int nxbin, int nybin)
{
  if (object != NULL)
  {
    return object->GetBinContent(nxbin, nybin);
  }
  else return 0;

}


void EmuMonitoringObject::SetAxisRange(double xmin, double xmax, std::string options)
{
  if (object != NULL)
  {
    object->SetAxisRange(xmin, xmax, options.c_str());
  }

}



int EmuMonitoringObject::setParameters(std::map<std::string, std::string> newparams, bool resetParams)
{
  std::map<std::string, std::string>::iterator itr;
  if (resetParams)
  {
    params.clear();
    params = newparams;
  }
  else
  {
    // == Append to parameters list
    for (itr = newparams.begin(); itr != newparams.end(); ++itr)
    {
      params[itr->first] = itr->second;
    }
  }
  return 0;
}

std::string EmuMonitoringObject::getParameter(std::string paramname)
{
  std::map<std::string, std::string>::iterator itr;
  if ((itr = params.find(paramname)) != params.end())
    return itr->second;
  else
    return "";
}

int EmuMonitoringObject::Fill(double xval)
{
  if (object != NULL)
  {
    // std::cout << name << ":" << xval << std::endl;
    return object->Fill(xval);
  }
  else return 1;

}

int EmuMonitoringObject::Fill(double xval, double yval)
{
  int res = 1;
  if (object != NULL)
  {
    //    if (std::string(object->ClassName()).find("TH2") != std::string::npos)
    // std::cout << name << ":" << xval << ":" << yval << std::endl;
    return object->Fill(xval, yval);
  }
  return res;

}

int EmuMonitoringObject::Fill(double xval, double yval, double zval)
{
  int res = 1;
  if (object != NULL)
  {
    if (std::string(object->ClassName()).find("TH3") != std::string::npos)
      return reinterpret_cast<TH3*>(object)->Fill(xval, yval, zval);
  }
  return res;
}

int EmuMonitoringObject::Fill(double xval, double yval, double zval, double wval)
{
  int res = 1;
  if (object != NULL)
  {
    if (std::string(object->ClassName()).find("TH3") != std::string::npos)
      return reinterpret_cast<TH3*>(object)->Fill(xval, yval, zval, wval);
  }
  return res;

}


int EmuMonitoringObject::parseDOMNode(DOMNode* info)
{
  std::map<std::string, std::string> obj_info;
  std::map<std::string, std::string>::iterator itr;
  DOMNodeList *children = info->getChildNodes();
  for (unsigned int i=0; i<children->getLength(); i++)
  {
    char * pChar = XMLString::transcode(children->item(i)->getNodeName());
    std::string paramname = std::string(pChar);
    delete[] pChar;
    if ( children->item(i)->hasChildNodes() )
    {
      char * pChar = XMLString::transcode(children->item(i)->getFirstChild()->getNodeValue());
      std::string param = std::string(pChar);
      obj_info[paramname] = param;
      delete[] pChar;
    }
  }

  if (obj_info.size() > 0)
  {
    // == Construct Monitoring Object Name
    std::string objname = "";
    if ((itr = obj_info.find("Type")) != obj_info.end())
    {

      objname += itr->second;
      type = itr->second;
      //  obj_info.erase("Type");

    }
    if ((itr = obj_info.find("Prefix")) != obj_info.end())
    {
      objname += itr->second;
      prefix = itr->second;
      //   obj_info.erase("Prefix");
    }
    if ((itr = obj_info.find("Name")) != obj_info.end())
    {
      objname += itr->second;
      name = itr->second;
      //   obj_info.erase("Name");
    }

    // == Get Monitoring Object Title
    if ((itr = obj_info.find("Title")) != obj_info.end())
    {
      title = itr->second;
      //   obj_info.erase("Title");
    }

    if ((itr = obj_info.find("Folder")) != obj_info.end())
    {
      folder = itr->second;
      //    obj_info.erase("Folder");
    }

    // == Create Monitoring Object Parameters map
    params.clear();
    for (itr = obj_info.begin(); itr != obj_info.end(); ++itr)
    {
      params[itr->first] = itr->second;
    }
  }
  return 0;
}

/**
 * @brief  Apply Monitoring Element Parameters. Those are mostly visual properties that format monitoring element.
 * @param
 * @return void
 */
void EmuMonitoringObject::applyPadProperties()
{
  char *stopstring;

  std::string leftMargin = getParameter("SetLeftMargin");
  if (leftMargin != "" )
  {
    gPad->SetLeftMargin(atof(leftMargin.c_str()));
  }

  std::string rightMargin = getParameter("SetRightMargin");
  if (rightMargin != "" )
  {
    gPad->SetRightMargin(atof(rightMargin.c_str()));
  }

  std::string logx = getParameter("SetLogx");
  if (logx!= "")
  {
    gPad->SetLogx();
  }

  std::string logy = getParameter("SetLogy");
  if (logy!= "" && (getObject()->GetMaximum()>0.))
  {
    gPad->SetLogy();
  }

  std::string logz = getParameter("SetLogz");
  if (logz!= "" && (getObject()->GetMaximum()>0.) )
  {
    gPad->SetLogz();
  }

  std::string gridx = getParameter("SetGridx");
  if (gridx!= "" )
  {
    gPad->SetGridx();
  }

  std::string gridy = getParameter("SetGridy");
  if (gridy!= "" )
  {
    gPad->SetGridy();
  }

  if (getParameter("SetStats") != "")
  {
    int stats = strtol( getParameter("SetStats").c_str(), &stopstring, 10 );
    getObject()->SetStats(bool(stats));
  }

}

