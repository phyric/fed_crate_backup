/*****************************************************************************\
* $Id: AppsVersion.h,v 1.36 2009/10/12 15:12:04 paste Exp $
\*****************************************************************************/

#ifndef __EMU_FED_APPSVERSION_H__
#define __EMU_FED_APPSVERSION_H__

#include "config/PackageInfo.h"

namespace EmuFEDApps {
	const std::string package     = "EmuFEDApps";
	const std::string versions    = "07.11.00";
	const std::string description = "Emu (CSC) FED XDAQ Applications";

	const std::string summary     = "Applications to control and monitor the Front End Device (FED) crates for the CMS Emu (CSC) subdetector";
	const std::string authors     = "Phillip Killewald, Stan Durkin, Jason Gilmore, Jianhui Gu";
	const std::string link        = "http://www.physics.ohio-state.edu/~cms";

	config::PackageInfo getPackageInfo();

	void checkPackageDependencies()
	throw (config::PackageInfo::VersionException);

	std::set<std::string, std::less<std::string> > getPackageDependencies();
}

#endif

