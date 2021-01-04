#pragma once

//Material texture names
#define ALBEDO_MAP "albedoMap"
#define NORMAL_MAP "normalMap"
#define MROT_MAP "mrotMap"
#define DISPLACEMENT_MAP "displacementMap"
#define METALLIC_MAP "metallicMap"
#define ROUGHNESS_MAP "roughnessMap"
#define OCCLUSION_MAP "occlusionMap"
#define EMISSION_MAP "emissionMap"
#define ALPHA_MAP "alphaMap"

//Specific textures' postfixes to be able to identify which texture contains which data
#define ALBEDO_MAP_POSTFIX "_DIF"
#define NORMAL_MAP_POSTFIX "_NRM"
#define MROT_MAP_POSTFIX "_MROT"
#define DISPLACEMENT_MAP_POSTFIX "_DISP"
#define METALLIC_MAP_POSTFIX "_MET"
#define ROUGHNESS_MAP_POSTFIX "_RGH"
#define OCCLUSION_MAP_POSTFIX "_OCC"
#define EMISSION_MAP_POSTFIX "_EMI"
#define ALPHA_MAP_POSTFIX "_A"

//Material constant uniform names
#define COLOUR "colour"
#define USES_NORMAL_MAP "usesNormalMap"
#define MROT "mrot"
#define HEIGHT_SCALE "heightScale"
#define HORIZONTAL_SCALE "horizontalScale"
#define METALLIC "metallic"
#define ROUGHNESS "roughness"
#define OCCLUSION "ambientOcclusion"
#define EMISSION "emission"
#define ALPHA "alpha"

//Component names:
#define RENDERER_COMPONENT "Renderer"
#define WIREFRAME_RENDERER_COMPONENT "WireframeRenderer"
#define LIGHT_COMPONENT "Light"
#define AUDIO_COMPONENT "Audio"

#define GUI_ELEMENT_COMPONENT "GUIElement"
#define GUI_BUTTON_COMPONENT "GUIButton"