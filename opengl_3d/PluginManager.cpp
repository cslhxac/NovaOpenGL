#include "PluginManager.h"
#include "ApplicationFactory.h"
#include <exception>
#include <iostream>
#include <utility>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

Nova::PluginManager::PluginManager(ApplicationFactory& app) : _app(app)
{
  ClearSearchPaths();
}

void 
Nova::PluginManager::ClearSearchPaths()
{
  _search_paths.clear();
  _search_paths.push_back("");
  _search_paths.push_back("./");
}


void 
Nova::PluginManager::SetSearchPaths(std::vector<std::string> paths)
{
    _search_paths = paths;
}

void 
Nova::PluginManager::AppendSearchPaths(std::vector<std::string> paths)
{
  _search_paths.insert( _search_paths.end(), paths.begin(), paths.end() );
}


void
Nova::PluginManager::LoadPlugin( std::string name ){
    auto res = _plugins.find( name );
    if( res == _plugins.end() ){
        //std::cout << "Requested plugin '"<< name << "' not loaded. Searching for plugin." << std::endl;
        
        for( auto p : _search_paths ){
            fs::path potential_path;
            potential_path /= p;
            potential_path /= name;
      
            //std::cout << "Trying to load '" << potential_path.native() << "'"<< std::endl;
      
            std::unique_ptr<Plugin> newPlugin;
            // First try to load the plugin
            try{
                newPlugin = std::unique_ptr<Plugin>( new Plugin(potential_path.native()) );
            }
            catch( std::exception& e ){
                std::cout << "Plugin '"<< potential_path.native() <<"' failed to load: " << e.what() << std::endl;
                continue;
            }
      
            // Check to match kernel version
            if( newPlugin->getEngineVersion() != Nova::API_VERSION ){
                std::cout << "Plugin API version mismatch. Plugin cannot be loaded." << std::endl;
                continue;
            }
      
            // Register the plugin and add it to the list
            newPlugin->registerPlugin( _app );
            std::cout << "Plugin " << name << " was loaded successfully." << std::endl;
            _plugins.insert( std::make_pair( name, std::move(newPlugin) ) );
            return;
        }
    }
}
