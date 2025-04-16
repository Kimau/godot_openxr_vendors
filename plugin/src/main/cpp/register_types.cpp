/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "export/export_plugin.h"
#include "export/khronos_export_plugin.h"
#include "export/lynx_export_plugin.h"
#include "export/magicleap_export_plugin.h"
#include "export/meta_export_plugin.h"
#include "export/pico_export_plugin.h"

#include "extensions/openxr_fb_android_surface_swapchain_create_extension_wrapper.h"
#include "extensions/openxr_fb_body_tracking_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_alpha_blend_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_depth_test_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_image_layout_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_secure_content_extension_wrapper.h"
#include "extensions/openxr_fb_composition_layer_settings_extension_wrapper.h"
#include "extensions/openxr_fb_face_tracking_extension_wrapper.h"
#include "extensions/openxr_fb_hand_tracking_aim_extension_wrapper.h"
#include "extensions/openxr_fb_hand_tracking_capsules_extension_wrapper.h"
#include "extensions/openxr_fb_hand_tracking_mesh_extension_wrapper.h"
#include "extensions/openxr_fb_passthrough_extension_wrapper.h"
#include "extensions/openxr_fb_render_model_extension_wrapper.h"
#include "extensions/openxr_fb_scene_capture_extension_wrapper.h"
#include "extensions/openxr_fb_scene_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_container_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_query_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_sharing_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_storage_batch_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_storage_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_user_extension_wrapper.h"
#include "extensions/openxr_htc_facial_tracking_extension_wrapper.h"
#include "extensions/openxr_htc_passthrough_extension_wrapper.h"
#include "extensions/openxr_meta_recommended_layer_resolution_extension_wrapper.h"
#include "extensions/openxr_meta_spatial_entity_mesh_extension_wrapper.h"

#include "classes/openxr_fb_hand_tracking_mesh.h"
#include "classes/openxr_fb_passthrough_geometry.h"
#include "classes/openxr_fb_render_model.h"
#include "classes/openxr_fb_scene_manager.h"
#include "classes/openxr_fb_spatial_anchor_manager.h"
#include "classes/openxr_fb_spatial_entity.h"
#include "classes/openxr_fb_spatial_entity_batch.h"
#include "classes/openxr_fb_spatial_entity_query.h"
#include "classes/openxr_fb_spatial_entity_user.h"
#include "classes/openxr_hybrid_app.h"
#include "classes/openxr_meta_passthrough_color_lut.h"

using namespace godot;


static void define_global_bool(ProjectSettings *project_settings, const String &p_name, bool p_default_value) {
	if (!project_settings->has_setting(p_name)) {
		project_settings->set_setting(p_name, p_default_value);
	}

	project_settings->set_initial_value(p_name, p_default_value);
	project_settings->set_as_basic(p_name, false);
	Dictionary property_info;
	property_info["name"] = p_name;
	property_info["type"] = Variant::Type::BOOL;
	property_info["hint"] = PROPERTY_HINT_NONE;
	project_settings->add_property_info(property_info);
}

static bool check_global_bool(ProjectSettings *project_settings, const String &p_name, bool p_default_value) {
    if (!project_settings->has_setting(p_name)) {
        return p_default_value;
    }

    return project_settings->get_setting(p_name);
}

static void add_plugin_core_settings(ProjectSettings *project_settings) {
    if (project_settings == nullptr) {
        return;
    }

    define_global_bool(project_settings, "xr/xrvendor/hybrid_app", false);

    // Meta/FB Base Features
    define_global_bool(project_settings, "xr/xrvendor/meta/body_tracking", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/face_tracking", false);

    // Meta/FB Hand Tracking Suite
    define_global_bool(project_settings, "xr/xrvendor/meta/hand_tracking/aim", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/hand_tracking/capsules", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/hand_tracking/mesh", false);

    // Meta/FB Composition & Rendering
    define_global_bool(project_settings, "xr/xrvendor/meta/composition/alpha_blend", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/composition/secure_content", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/composition/layer_settings", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/recommended_layer_resolution", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/android_surface_swapchain", false);

    // Meta/FB Scene & Spatial Features
    define_global_bool(project_settings, "xr/xrvendor/meta/spatial/core", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/spatial/sharing", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/spatial/storage_batch", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/spatial/user", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/spatial/mesh", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/scene/capture", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/scene/core", false);

    // Meta/FB Passthrough & Models
    define_global_bool(project_settings, "xr/xrvendor/meta/passthrough", false);
    define_global_bool(project_settings, "xr/xrvendor/meta/render_model", false);

    // HTC Extensions
    define_global_bool(project_settings, "xr/xrvendor/htc/facial_tracking", false);
    define_global_bool(project_settings, "xr/xrvendor/htc/passthrough", false);
}


void add_plugin_project_settings(ProjectSettings *project_settings) {
	{
		// Add the 'automatically_request_runtime_permissions' project setting
		String request_permissions_setting = "xr/openxr/extensions/automatically_request_runtime_permissions";
		if (!project_settings->has_setting(request_permissions_setting)) {
			// Default value is `true` to match prior plugin behavior
			project_settings->set_setting(request_permissions_setting, true);
		}

		project_settings->set_initial_value(request_permissions_setting, true);
		project_settings->set_as_basic(request_permissions_setting, false);
		Dictionary property_info;
		property_info["name"] = request_permissions_setting;
		property_info["type"] = Variant::Type::BOOL;
		property_info["hint"] = PROPERTY_HINT_NONE;
		project_settings->add_property_info(property_info);
	}

	{
		String collision_shape_2d_thickness = "xr/openxr/extensions/meta_scene_api/collision_shape_2d_thickness";
		if (!project_settings->has_setting(collision_shape_2d_thickness)) {
			project_settings->set_setting(collision_shape_2d_thickness, 0.1);
		}

		project_settings->set_initial_value(collision_shape_2d_thickness, 0.1);
		project_settings->set_as_basic(collision_shape_2d_thickness, false);
		Dictionary property_info;
		property_info["name"] = collision_shape_2d_thickness;
		property_info["type"] = Variant::Type::FLOAT;
		property_info["hint"] = PROPERTY_HINT_NONE;
		project_settings->add_property_info(property_info);
	}

	{
		String hybrid_app_enabled_setting = "xr/hybrid_app/enabled";
		if (!project_settings->has_setting(hybrid_app_enabled_setting)) {
			project_settings->set_setting(hybrid_app_enabled_setting, false);
		}

		project_settings->set_initial_value(hybrid_app_enabled_setting, false);
		project_settings->set_as_basic(hybrid_app_enabled_setting, true);
		Dictionary hybrid_app_enabled_property_info;
		hybrid_app_enabled_property_info["name"] = hybrid_app_enabled_setting;
		hybrid_app_enabled_property_info["type"] = Variant::Type::BOOL;
		hybrid_app_enabled_property_info["hint"] = PROPERTY_HINT_NONE;
		project_settings->add_property_info(hybrid_app_enabled_property_info);

		String hybrid_app_launch_mode_setting = "xr/hybrid_app/launch_mode";
		if (!project_settings->has_setting(hybrid_app_launch_mode_setting)) {
			project_settings->set_setting(hybrid_app_launch_mode_setting, OpenXRHybridApp::HYBRID_MODE_IMMERSIVE);
		}

		project_settings->set_initial_value(hybrid_app_launch_mode_setting, OpenXRHybridApp::HYBRID_MODE_IMMERSIVE);
		project_settings->set_as_basic(hybrid_app_launch_mode_setting, true);
		Dictionary hybrid_app_launch_mode_property_info;
		hybrid_app_launch_mode_property_info["name"] = hybrid_app_launch_mode_setting;
		hybrid_app_launch_mode_property_info["type"] = Variant::Type::INT;
		hybrid_app_launch_mode_property_info["hint"] = PROPERTY_HINT_ENUM;
		hybrid_app_launch_mode_property_info["hint_string"] = "Start As Immersive:0,Start As Panel:1";
		project_settings->add_property_info(hybrid_app_launch_mode_property_info);
	}
}


void initialize_plugin_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE: {
			ProjectSettings *project_settings = ProjectSettings::get_singleton();
			add_plugin_core_settings(project_settings);

			ClassDB::register_class<OpenXRFbPassthroughExtensionWrapper>();			
			ClassDB::register_class<OpenXRFbRenderModelExtensionWrapper>();			
			ClassDB::register_class<OpenXRFbSceneCaptureExtensionWrapper>();			
			ClassDB::register_class<OpenXRFbSpatialEntityExtensionWrapper>();			
			ClassDB::register_class<OpenXRFbSpatialEntitySharingExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityStorageExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityStorageBatchExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityQueryExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityContainerExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSpatialEntityUserExtensionWrapper>();
			ClassDB::register_class<OpenXRMetaRecommendedLayerResolutionExtensionWrapper>();
			ClassDB::register_class<OpenXRMetaSpatialEntityMeshExtensionWrapper>();
			ClassDB::register_class<OpenXRFbSceneExtensionWrapper>();
			ClassDB::register_class<OpenXRFbFaceTrackingExtensionWrapper>();
			ClassDB::register_class<OpenXRFbBodyTrackingExtensionWrapper>();
			ClassDB::register_class<OpenXRFbHandTrackingMeshExtensionWrapper>();
			ClassDB::register_class<OpenXRFbHandTrackingAimExtensionWrapper>();
			ClassDB::register_class<OpenXRFbHandTrackingCapsulesExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerSecureContentExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerDepthTestExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerAlphaBlendExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerImageLayoutExtensionWrapper>();
			ClassDB::register_class<OpenXRFbCompositionLayerSettingsExtensionWrapper>();
			ClassDB::register_class<OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper>();
			ClassDB::register_class<OpenXRHtcFacialTrackingExtensionWrapper>();
			ClassDB::register_class<OpenXRHtcPassthroughExtensionWrapper>();

			// Only init the ones that are setup
			if (project_settings) {
				// Meta/FB Features
				if (check_global_bool(project_settings, "xr/xrvendor/meta/passthrough", false))
					OpenXRFbPassthroughExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/render_model", false))
					OpenXRFbRenderModelExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/scene/capture", false))
					OpenXRFbSceneCaptureExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				// Spatial Entity Core Extensions
				if (check_global_bool(project_settings, "xr/xrvendor/meta/spatial/core", false)) {
					OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->register_extension_wrapper();
					OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton()->register_extension_wrapper();
					OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->register_extension_wrapper();
					OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton()->register_extension_wrapper();
				}
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/spatial/sharing", false))
					OpenXRFbSpatialEntitySharingExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/spatial/storage_batch", false))
					OpenXRFbSpatialEntityStorageBatchExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/spatial/user", false))
					OpenXRFbSpatialEntityUserExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/recommended_layer_resolution", false))
					OpenXRMetaRecommendedLayerResolutionExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/spatial/mesh", false))
					OpenXRMetaSpatialEntityMeshExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/scene/core", false))
					OpenXRFbSceneExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/face_tracking", false))
					OpenXRFbFaceTrackingExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/body_tracking", false))
					OpenXRFbBodyTrackingExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				// Hand tracking extensions
				if (check_global_bool(project_settings, "xr/xrvendor/meta/hand_tracking/mesh", false))
					OpenXRFbHandTrackingMeshExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/hand_tracking/aim", false))
					OpenXRFbHandTrackingAimExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/hand_tracking/capsules", false))
					OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/composition/secure_content", false))
					OpenXRFbCompositionLayerSecureContentExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				// Composition layer settings group
				if (check_global_bool(project_settings, "xr/xrvendor/meta/composition/layer_settings", false)) {
					OpenXRFbCompositionLayerDepthTestExtensionWrapper::get_singleton()->register_extension_wrapper();
					OpenXRFbCompositionLayerImageLayoutExtensionWrapper::get_singleton()->register_extension_wrapper();
					OpenXRFbCompositionLayerSettingsExtensionWrapper::get_singleton()->register_extension_wrapper();
				}
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/composition/alpha_blend", false))
					OpenXRFbCompositionLayerAlphaBlendExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/meta/android_surface_swapchain", false))
					OpenXRFbAndroidSurfaceSwapchainCreateExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				// HTC extensions
				if (check_global_bool(project_settings, "xr/xrvendor/htc/facial_tracking", false))
					OpenXRHtcFacialTrackingExtensionWrapper::get_singleton()->register_extension_wrapper();
					
				if (check_global_bool(project_settings, "xr/xrvendor/htc/passthrough", false))
					OpenXRHtcPassthroughExtensionWrapper::get_singleton()->register_extension_wrapper();
			}
		} break;

		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;

		case MODULE_INITIALIZATION_LEVEL_SCENE: {
			ProjectSettings *project_settings = ProjectSettings::get_singleton();
			if (project_settings == nullptr) {
				return;
			}

			add_plugin_project_settings(project_settings);

			ClassDB::register_class<OpenXRFbRenderModel>();
			ClassDB::register_class<OpenXRFbHandTrackingMesh>();
			ClassDB::register_class<OpenXRFbSceneManager>();
			ClassDB::register_class<OpenXRFbSpatialAnchorManager>();
			ClassDB::register_class<OpenXRFbSpatialEntity>();
			ClassDB::register_class<OpenXRFbSpatialEntityBatch>();
			ClassDB::register_class<OpenXRFbSpatialEntityQuery>();
			ClassDB::register_class<OpenXRFbSpatialEntityUser>();
			ClassDB::register_class<OpenXRFbPassthroughGeometry>();
			ClassDB::register_class<OpenXRMetaPassthroughColorLut>();
			ClassDB::register_class<OpenXRHybridApp>();

			Engine* eng = Engine::get_singleton();

			// Only register singletons if corresponding settings are enabled
			if (check_global_bool(project_settings, "xr/xrvendor/meta/passthrough", false))
				eng->register_singleton("OpenXRFbPassthroughExtensionWrapper", OpenXRFbPassthroughExtensionWrapper::get_singleton());

			if (check_global_bool(project_settings, "xr/xrvendor/meta/render_model", false))
				eng->register_singleton("OpenXRFbRenderModelExtensionWrapper", OpenXRFbRenderModelExtensionWrapper::get_singleton());

			if (check_global_bool(project_settings, "xr/xrvendor/meta/scene/capture", false))
				eng->register_singleton("OpenXRFbSceneCaptureExtensionWrapper", OpenXRFbSceneCaptureExtensionWrapper::get_singleton());

			if (check_global_bool(project_settings, "xr/xrvendor/meta/spatial/core", false)) {
				eng->register_singleton("OpenXRFbSpatialEntityExtensionWrapper", OpenXRFbSpatialEntityExtensionWrapper::get_singleton());
				eng->register_singleton("OpenXRFbSpatialEntityStorageExtensionWrapper", OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton());
				eng->register_singleton("OpenXRFbSpatialEntityQueryExtensionWrapper", OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton());
				eng->register_singleton("OpenXRFbSpatialEntityContainerExtensionWrapper", OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton());
				eng->register_singleton("OpenXRFbSceneExtensionWrapper", OpenXRFbSceneExtensionWrapper::get_singleton());
			}

			if (check_global_bool(project_settings, "xr/xrvendor/meta/hand_tracking/aim", false))
				eng->register_singleton("OpenXRFbHandTrackingAimExtensionWrapper", OpenXRFbHandTrackingAimExtensionWrapper::get_singleton());

			if (check_global_bool(project_settings, "xr/xrvendor/meta/hand_tracking/capsules", false))
				eng->register_singleton("OpenXRFbHandTrackingCapsulesExtensionWrapper", OpenXRFbHandTrackingCapsulesExtensionWrapper::get_singleton());

			if (check_global_bool(project_settings, "xr/xrvendor/meta/composition/layer_settings", false)) {
				eng->register_singleton("OpenXRFbCompositionLayerDepthTestExtensionWrapper", OpenXRFbCompositionLayerSettingsExtensionWrapper::get_singleton());
				eng->register_singleton("OpenXRFbCompositionLayerSettingsExtensionWrapper", OpenXRFbCompositionLayerSettingsExtensionWrapper::get_singleton());
			}

			if (check_global_bool(project_settings, "xr/xrvendor/htc/facial_tracking", false))
				eng->register_singleton("OpenXRHtcFacialTrackingExtensionWrapper", OpenXRHtcFacialTrackingExtensionWrapper::get_singleton());

			if (check_global_bool(project_settings, "xr/xrvendor/htc/passthrough", false))
				eng->register_singleton("OpenXRHtcPassthroughExtensionWrapper", OpenXRHtcPassthroughExtensionWrapper::get_singleton());

			// Hybrid app is a special case with its own setting
			if (check_global_bool(project_settings, "xr/xrvendor/hybrid_app", false))
				eng->register_singleton("OpenXRHybridApp", OpenXRHybridApp::get_singleton());

			
			OpenXRFbHandTrackingAimExtensionWrapper::add_project_setting(project_settings);
			OpenXRMetaRecommendedLayerResolutionExtensionWrapper::add_project_setting(project_settings);

		} break;

		case MODULE_INITIALIZATION_LEVEL_EDITOR: {
			ClassDB::register_class<OpenXREditorExportPlugin>();

			ClassDB::register_class<KhronosEditorExportPlugin>();
			ClassDB::register_class<KhronosEditorPlugin>();
			EditorPlugins::add_by_type<KhronosEditorPlugin>();

			ClassDB::register_class<LynxEditorPlugin>();
			EditorPlugins::add_by_type<LynxEditorPlugin>();

			ClassDB::register_class<MetaEditorExportPlugin>();
			ClassDB::register_class<MetaEditorPlugin>();
			EditorPlugins::add_by_type<MetaEditorPlugin>();

			ClassDB::register_class<PicoEditorExportPlugin>();
			ClassDB::register_class<PicoEditorPlugin>();
			EditorPlugins::add_by_type<PicoEditorPlugin>();

			ClassDB::register_class<MagicleapEditorExportPlugin>();
			ClassDB::register_class<MagicleapEditorPlugin>();
			EditorPlugins::add_by_type<MagicleapEditorPlugin>();
		} break;

		case MODULE_INITIALIZATION_LEVEL_MAX:
			break;
	}
}

void terminate_plugin_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE:
			break;

		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;

		case MODULE_INITIALIZATION_LEVEL_SCENE: {
			Engine* eng = Engine::get_singleton();

			eng->unregister_singleton("OpenXRFbPassthroughExtensionWrapper");
			eng->unregister_singleton("OpenXRFbRenderModelExtensionWrapper");
			eng->unregister_singleton("OpenXRFbSceneCaptureExtensionWrapper");
			eng->unregister_singleton("OpenXRFbSpatialEntityExtensionWrapper");
			eng->unregister_singleton("OpenXRFbSpatialEntityStorageExtensionWrapper");
			eng->unregister_singleton("OpenXRFbSpatialEntityQueryExtensionWrapper");
			eng->unregister_singleton("OpenXRFbSpatialEntityContainerExtensionWrapper");
			eng->unregister_singleton("OpenXRFbSceneExtensionWrapper");
			eng->unregister_singleton("OpenXRFbHandTrackingAimExtensionWrapper");
			eng->unregister_singleton("OpenXRFbHandTrackingCapsulesExtensionWrapper");
			eng->unregister_singleton("OpenXRFbCompositionLayerDepthTestExtensionWrapper");
			eng->unregister_singleton("OpenXRFbCompositionLayerSettingsExtensionWrapper");
			eng->unregister_singleton("OpenXRHtcFacialTrackingExtensionWrapper");
			eng->unregister_singleton("OpenXRHtcPassthroughExtensionWrapper");
			eng->unregister_singleton("OpenXRHybridApp");

			memdelete(OpenXRHybridApp::get_singleton());
		} break;

		case MODULE_INITIALIZATION_LEVEL_EDITOR:
			break;

		case MODULE_INITIALIZATION_LEVEL_MAX:
			break;
	}
}


extern "C" {
GDExtensionBool GDE_EXPORT plugin_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_plugin_module);
	init_obj.register_terminator(terminate_plugin_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
