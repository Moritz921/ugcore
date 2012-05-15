// created by Sebastian Reiter
// s.b.reiter@googlemail.com
// 22.03.2011 (m,d,y)

#include "common/util/path_provider.h"
#include "registry/registry.h"
#include "bridge.h"
#include "common/util/file_util.h"
#include "ug.h"

#include <cstdlib>
#include <string>

using namespace std;

namespace ug{
namespace bridge{

static string GetAppPath()
{return PathProvider::get_path(APP_PATH);}

static string GetDataPath()
{return PathProvider::get_path(DATA_PATH);}

static string GetScriptPath()
{return PathProvider::get_path(SCRIPT_PATH);}

static string GetCurrentPath()
{return PathProvider::get_current_path();}

/**
 * This function executes the given string in a system shell (if available)
 *
 * \param[in]	cmd		The shell command to execute
 * \returns 	1 		if no shell available
 */
static int ExecuteSystemCommand(const char* cmd)
{
//	check that some shell exists
	if(!system(NULL)) return 1;

//	run the command
	return system(cmd);
}

void int_srand(int seed)
{
	srand((unsigned int)seed);
}

bool RegisterUtilInterface(Registry& reg, string parentGroup)
{
	string grp(parentGroup);
	grp.append("/Util");

	reg.add_function("ug_get_app_path", &GetAppPath, grp,
					 "pathName", "", "Returns the application path");

	reg.add_function("ug_get_data_path", &GetDataPath, grp,
					 "pathName", "", "Returns the data path");

	reg.add_function("ug_get_script_path", &GetScriptPath, grp,
					 "pathName", "", "Returns the script path");

	reg.add_function("ug_get_current_path", &GetCurrentPath, grp,
					 "pathName", "", "Returns the current path");

	reg.add_function("ExecuteSystemCommand", &ExecuteSystemCommand, grp,
					 "success", "command", "Executes a command in the system shell");

	reg.add_function("srand", int_srand, grp, "seed", "The pseudo-random number generator is initialized using the argument passed as seed.")
		.add_function("ug_file_exists", &FileExists, grp,
				 "exists", "", "Returns true if a path exists, false if not.")
		.add_function("exit", &UGForceExit, grp,
				 "", "", "Immediatly terminates the application.")
		.add_function("quit", &UGForceExit, grp,
				 "", "", "Immediatly terminates the application.");

	return true;
}

}// end of namespace bridge
}// end of namespace ug
