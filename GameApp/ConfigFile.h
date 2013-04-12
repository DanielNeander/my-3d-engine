#pragma once 


#include <string>
#include <map>
#include <list>
using namespace std;

class ConfigFile
{
    public:

        ConfigFile(string configFile, char** envp = 0);
        virtual ~ConfigFile();

		// get string config entry
		string pString(string name);

		/* get boolean config entry
		 * A value of Yes/yes/YES/true/True/TRUE leads to true,
		 * all other values leads to false.
		 */
		bool pBool(string name);

		// get double config entry; value is parsed using atof()
		double pDouble(string name);

		float pFloat(string name);

		// get int config entry; value is parsed using atoi()
		int pInt(string name);

		// get the symbol map (e.g. for iterating over all symbols)
		inline map<string, string>& getSymbols() {
			return symbols;
		}

		// get config sub group
		inline ConfigFile* group(string name) {
			return groups[name];
		}

		// get config sub group map (e.g. for iterating over all groups)
		inline map<string, ConfigFile*>& getGroups() {
			return groups;
		}

	private:
		// private constructor for sub groups
		ConfigFile(string name, string parentDebugInfo);

		// helper functions for parsing
		void add(string name, string value);
		void split(string in, string& left, string& right, char c);
		void trim(string& s);
		void symbolExpand(string& s);
		void symbolExpand(map<string, string>& symbols, string& s);
		void envSymbolExpand(string& s);
		
		// config group symbol map
		map<string, string> symbols;

		// environment symbol map
		map<string, string> envSymbols;

		// config sub group map
		map<string, ConfigFile*> groups;

		// stack of config groups for parsing (only used in top config element)
		list<ConfigFile*> groupStack;

		// debug info used for logging messages
		string debugInfo;
        
    };