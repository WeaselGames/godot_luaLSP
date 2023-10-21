#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "luaCodeEdit.h"
#include "luaHighlighter.h"
#include "luaLanguageServer.h"

using namespace godot;

void initialize_luaCodeEdit_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<LuaLanguageServer>();
	ClassDB::register_class<LuaHighlighter>();
	ClassDB::register_class<LuaCodeEdit>();
}

void uninitialize_luaCodeEdit_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
// Initialization.

GDExtensionBool GDE_EXPORT luaCodeEdit_library_init(GDExtensionInterfaceGetProcAddress p_interface, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

	init_obj.register_initializer(initialize_luaCodeEdit_module);
	init_obj.register_terminator(uninitialize_luaCodeEdit_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}