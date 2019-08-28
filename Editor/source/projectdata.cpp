#include "projectdata.h"
#include <QFile>
#include <QDir>
#include <sstream>

ProjectData::ProjectData() {
  create_new_project();
}

ProjectData::~ProjectData() {
  clear_database();
}

void ProjectData::create_new_project() {
  // Create a new, clean project document.
  project_xml_document.reset();

  project_xml_document.append_child(pugi::xml_node_type::node_comment).set_value("Generated by Lumi Game Studio");
  project_xml_document.append_child(pugi::xml_node_type::node_comment).set_value("EDITING THIS FILE BY HAND CAN BREAK YOUR GAME. Proceed with caution.");

  pugi::xml_node root_node = project_xml_document.append_child("project");

  for(std::string child_name : {"backgrounds", "objects", "sprites", "sounds", "tilesets", "rooms"})
    root_node.append_child(child_name.c_str());

  config_node = root_node.append_child("window");
  config_node.append_attribute("width").set_value(320);
  config_node.append_attribute("height").set_value(240);
  config_node.append_attribute("scale").set_value(2);
  config_node.append_attribute("drawcolor").set_value("000000");
  config_node.append_attribute("title").set_value("My Lumi Game");
  config_node.append_attribute("fps").set_value(60);
  config_node.append_attribute("defaultroom").set_value(""); // This is calculated when we export a game.
}

// Asset stuff
bool ProjectData::asset_name_exists(std::string name) {
  return name_to_asset_map.count(name) > 0;
}

void ProjectData::rename_asset(std::string old_name, std::string new_name) {
  AssetEntry* entry = name_to_asset_map.at(old_name);
  name_to_asset_map.erase(old_name);
  name_to_asset_map.insert({new_name, entry});
}

bool ProjectData::asset_id_exists(int id) {
  return asset_db.count(id) > 0;
}

AssetEntry* ProjectData::get_asset(int id) {
  if(!asset_id_exists(id)) return nullptr;
  return asset_db.at(id);
}



// Database stuff
void ProjectData::clear_database() {
  // delete all heap-allocated asset entries
    for(std::pair<int, AssetEntry*> kv : asset_db)
        delete kv.second;

  // clear maps
  asset_db.clear();
  name_to_asset_map.clear();

  // clear held project config node
  config_node = pugi::xml_node();
}

void ProjectData::load_entries_into_db(pugi::xml_node &root, QString subset_name, ASSET_TYPE type) {
  for(pugi::xml_node& node : root.child(subset_name.toUtf8().data()).children()){
      load_entry_into_db(node, type);
  }
}

void ProjectData::load_entry_into_db(pugi::xml_node &node, ASSET_TYPE type) {
  AssetEntry* asset_ptr = new AssetEntry;
  asset_ptr->id = generate_new_unique_id();
  asset_ptr->node = node;
  asset_ptr->type = type;
  asset_ptr->name = asset_ptr->name = node.attribute("name").as_string();
  asset_db.insert({asset_ptr->id, asset_ptr});
  name_to_asset_map.insert({asset_ptr->name, asset_ptr});
}

std::unordered_map<int, AssetEntry*>* ProjectData::get_map() {
  return &asset_db;
}


// Utilities
int ProjectData::generate_new_unique_id() {
  int id = static_cast<int>(asset_db.size());
  do id++; while (asset_db.count(id) > 0);
  return id;
}

pugi::xml_node* ProjectData::get_config_node() {
  return &config_node;
}


bool ProjectData::load_project_file_into_database(QString path) {
  // does nothing if user escapes the file dialog
  if(path.compare("") == 0) return false;

  clear_database();

  pugi::xml_parse_result result = project_xml_document.load_file(path.toUtf8().data());
  if (result) {
    printf("Parsed project %s without errors!\n", path.toUtf8().data());
  }else{
    printf("Error: %s at %lld", result.description(), result.offset);
    return false;
  }

  pugi::xml_node root_node = project_xml_document.child("project");

  // load project name
  game_name = root_node.child("name").text().as_string();

  // Load project into databases
  load_entries_into_db(root_node, "objects",     ASSET_TYPE::OBJECT);
  load_entries_into_db(root_node, "sounds",      ASSET_TYPE::SOUND);
  load_entries_into_db(root_node, "sprites",     ASSET_TYPE::SPRITE);
  load_entries_into_db(root_node, "tilesets",    ASSET_TYPE::TILESET);
  load_entries_into_db(root_node, "backgrounds", ASSET_TYPE::BACKGROUND);
  load_entries_into_db(root_node, "rooms",       ASSET_TYPE::ROOM);

  printf("Loaded database. Size: %lld\n", asset_db.size());

  // load config
  config_node = root_node.child("window");

  set_name_and_dir_from_path(path);

  return true;
}

/*
 * Saving:
 * - check if projectdata's save file location string is not empty
 * - check if projectdata's save file location exists on disk
 * if yes:
 * - overwrite the save file's location with new document (moves all temp files into save directory)
 * if not:
 * - show dialog asking for a new save location
 */
bool ProjectData::save_current_project_to_file(QString path) {

  std::string xml_file_data = get_project_xml_as_string();


  // Write .lumi XML file
  QFile file(path);
  file.open(QIODevice::ReadWrite);
  if(file.write(xml_file_data.c_str()) == -1) return false;
  file.close();

  // TODO: Copy all temp assets to the given directory


  // Set current project data to given
  set_name_and_dir_from_path(path);

  return true;
}

void ProjectData::set_name_and_dir_from_path(QString path) {
  current_loaded_file_name = path.split('/').last();
  current_loaded_file_directory = path.replace(current_loaded_file_name, "");
}

std::string ProjectData::get_project_xml_as_string() {
  // save file to stringstream
  std::stringstream ss;
  project_xml_document.save(ss, " ");
  return ss.str();
}

QString ProjectData::get_current_project_file_directory() {
  return current_loaded_file_directory;
}

QString ProjectData::get_current_project_file_name() {
  return current_loaded_file_name;
}

pugi::xml_document& ProjectData::get_project_xml_document() {
  return project_xml_document;
}
