#include <chrono>
#include <cstdint>
#include <random>
#include <vector>
#include "../yocto/yocto_gl.h"
using namespace ygl;

// note1 : obj files, mtl files and png files for textures must be all in the same folder!
// note2 : every object in the scene, must be stored in the proper vector 
// contained in the scene object(the pointer of it), in order to delete it when deleting the scene;

static const float min_map_side = 400.0f;
static const float max_map_side = 400.0f;
static const float min_building_side = 9.0f;
static const float max_building_side = 30.0f;
static const float min_street_width = 4.0f;
static const float max_street_width = 10.0f;

// the types of building
enum building_type { residential, skyscraper, house };

// the axis to make subdivisions
enum subdiv_axis { x, y };

// the type of element obtainable by subidiving facade. 
// Only the terminal elements will be added to the scene and textured
enum element_type {
	facade,
	topfloors,
	topfloor,
	wintiles,
	wintile,
	windcol,
	vwall,
	hwall,
	window,
	ledge,
	bottomfloor,
	doortile,
	doorcol,
	door,
	roof
};

// map building types with strings
static const std::map <building_type, std::string> building_type_to_string = {
	{ residential, "residential" },
	{ skyscraper, "skyscraper" },
	{ house, "house" },
};

// map subdivision axis with strings
static const std::map <subdiv_axis, std::string> axis_to_string = {
	{ x, "x" }, { y , "y" }
};

// map element types with strings
static const std::map <element_type, std::string> element_type_to_string = {
	{ facade, "facade" }, { topfloors, "topfloors" }, { topfloor, "topfloor" },
	{ wintiles, "wintiles" }, { windcol, "windcol" }, { vwall, " vwall" },
	{ hwall, "hwall" }, { window, "window" }, { ledge, "ledge" },
	{ bottomfloor, "bottomfloor" }, { doortile, "doortile" }, { door, "door" }, { roof, "roof" }
};

// contains parameters and rules for facades subdivision
// every building type has its own rules,
// defined by an instance of type_constants
struct type_constants {
	std::pair<float, float> h_range;
	std::pair<float, float> l_range;
	std::pair<float, float> w_range;
	std::pair<float, float> wl_ratio_range;
	std::pair<float, float> floor_h_range;
	std::pair<float, float> wind_w_range;
	std::pair<float, float> wind_h_range;
	std::map<element_type, std::pair<std::vector<element_type>, subdiv_axis>> subdiv_rules;
};

// constants for residential buildings
// constants are equal for all types, temporary
static const type_constants residential_constants = {
	std::pair<float, float>{ 30.0, 100.0 },
	std::pair<float, float>{ 4.0, 10.0 },
	std::pair<float, float>{ 4.0, 10.0 },
	std::pair<float, float>{ (5.0 / 6.0), (6.0 / 5.0) },
	std::pair<float, float>{ 2.5, 3.5 },
	std::pair<float, float>{ 2.0, 2.5 },
	std::pair<float, float>{ 2.0, 2.5 },
	std::map<element_type, std::pair<std::vector<element_type>, subdiv_axis>> {
		{ facade,     std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{bottomfloor, topfloors},      y} },
		{ topfloors,  std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{topfloor},                    y} },
		{ bottomfloor,std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintiles, doortile, wintiles},x} },
		{ topfloor,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintile},                     x} },
		{ wintiles,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintile},                     x} },
		{ wintile,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{vwall, windcol, vwall},       x} },
		{ windcol,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{hwall, window, hwall},        y} },
		{ doortile,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{hwall, doorcol, hwall},       x} },
		{ doorcol,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{door, vwall},                 y} }
	}
};

// constants for skyscrapers
// constants are equal for all types, temporary
static const type_constants skyscraper_constants = {
	std::pair<float, float>{ 30.0, 100.0 },
	std::pair<float, float>{ 4.0, 10.0 },
	std::pair<float, float>{ 4.0, 10.0 },
	std::pair<float, float>{ (5.0 / 6.0), (6.0 / 5.0) },
	std::pair<float, float>{ 2.5, 3.5 },
	std::pair<float, float>{ 2.0, 2.5 },
	std::pair<float, float>{ 2.0, 2.5 },
	std::map<element_type, std::pair<std::vector<element_type>, subdiv_axis>> {
		{ facade,     std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{bottomfloor, topfloors},      y} },
		{ topfloors,  std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{topfloor},                    y} },
		{ bottomfloor,std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintiles, doortile, wintiles},x} },
		{ topfloor,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintile},                     x} },
		{ wintiles,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintile},                     x} },
		{ wintile,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{vwall, windcol, vwall},       x} },
		{ windcol,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{hwall, window, hwall},        y} },
		{ doortile,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{hwall, doorcol, hwall},       x} },
		{ doorcol,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{door, vwall},                 y} }
	}
};

// constants for houses
// constants are equal for all types, temporary
static const type_constants house_info = {
	std::pair<float, float>{ 30.0, 100.0 },
	std::pair<float, float>{ 4.0, 10.0 },
	std::pair<float, float>{ 4.0, 10.0 },
	std::pair<float, float>{ (5.0 / 6.0), (6.0 / 5.0) },
	std::pair<float, float>{ 2.5, 3.5 },
	std::pair<float, float>{ 2.0, 2.5 },
	std::pair<float, float>{ 2.0, 2.5 },
	std::map<element_type, std::pair<std::vector<element_type>, subdiv_axis>> {
		{ facade,     std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{bottomfloor, topfloors},      y} },
		{ topfloors,  std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{topfloor},                    y} },
		{ bottomfloor,std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintiles, doortile, wintiles},x} },
		{ topfloor,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintile},                     x} },
		{ wintiles,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{wintile},                     x} },
		{ wintile,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{vwall, windcol, vwall},       x} },
		{ windcol,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{hwall, window, hwall},        y} },
		{ doortile,   std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{hwall, doorcol, hwall},       x} },
		{ doorcol,    std::pair<std::vector<element_type>, subdiv_axis>{std::vector<element_type>{door, vwall},                 y} }
	}
};

// contains the textures that can be applied to building elements
// every building type has its own textures
// defined by an instance of type_textures
struct type_textures {
	std::map<element_type, std::vector<std::string>> texture_pool;
};

// textures for residential buildings
// temporary empty
static const type_textures residential_textures = {
	std::map<element_type, std::vector<std::string>> {
		{ vwall, std::vector<std::string>{} },
		{ hwall, std::vector<std::string>{} },
		{ door,  std::vector<std::string>{} },
		{ window,std::vector<std::string>{} },
		{ roof,  std::vector<std::string>{} }
	}
};

// textures for skyscrapers
// temporary empty
static const type_textures skyscraper_textures = {
	std::map<element_type, std::vector<std::string>> {
		{ vwall, std::vector<std::string>{} },
		{ hwall, std::vector<std::string>{} },
		{ door,  std::vector<std::string>{} },
		{ window,std::vector<std::string>{} },
		{ roof,  std::vector<std::string>{} }
	}
};

// textures for houses
// temporary empty
static const type_textures house_textures = {
	std::map<element_type, std::vector<std::string>> {
		{ vwall, std::vector<std::string>{} },
		{ hwall, std::vector<std::string>{} },
		{ door,  std::vector<std::string>{} },
		{ window,std::vector<std::string>{} },
		{ roof,  std::vector<std::string>{} }
	}
};

//
struct building_info {
	building_type type;
	//type_constants info;
	float h;
	float l;
	float w;
	float w_l_ratio;
	float floor_h;
	float wind_w;
	float wind_h;
	bool flat_roof;
	//std::map<element_type, std::pair<std::vector<element_type>, subdiv_axis>> subdiv_rules;

	//building_info(building_type) {
		
	//}


	//~building_info() {

	//}
};

building_info prototype = { skyscraper, 10, 1, 1, 1, 3.5f, 1, 1, true };

//unused
struct building_inst {
	instance* building;
	building_info info;
};

/**
* Generate a randomic function that use the specifyed distribution
* It seeds the random generator with the tcurrent time as entropy
* Return : a distribution, generator binded function. Use as normal invocation. Voldemort type.
*/
auto bind_random_distribution(std::uniform_int_distribution<uint32_t> distrib) {  //TODO allow more distribution types
	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
	return  std::bind(distrib, generator);
}

//add instance
void add_instance(scene* scn, const std::string& name, const frame3f& f, shape* shp) {
	if (!shp) return;

	shape_group* shpgrp = new shape_group{};
	shpgrp->shapes.push_back(shp);

	instance* inst = new instance{ name, f, shpgrp };
	if (std::find(scn->instances.begin(), scn->instances.end(), inst) == scn->instances.end()) {
		scn->instances.push_back(inst);
	}
	if (std::find(scn->shapes.begin(), scn->shapes.end(), shpgrp) == scn->shapes.end()) {
		scn->shapes.push_back(shpgrp);
	}
}

// add the shape to the instance
void add_shape_in_instance(instance* inst , shape* shp) {
	inst->shp->shapes.push_back(shp);
}

// remove the shape from the scene
void remove_shape_from_scene(scene* scn, shape* shp) {
	if (shp->mat != nullptr) {
		auto res1 = std::find(scn->materials.begin(), scn->materials.end(), shp->mat);
		if (res1 != scn->materials.end()) {
			scn->materials.erase(res1);
			delete shp->mat->kd_txt;
		}
		auto res2 = std::find(scn->textures.begin(), scn->textures.end(), shp->mat->kd_txt);
		if (res2 != scn->textures.end()) {
			scn->textures.erase(res2);
			delete shp->mat;
		}
	}
	for(auto shps : scn->shapes) {
		auto res = std::find(shps->shapes.begin(), shps->shapes.end(), shp);
		if(res != shps->shapes.end()){
			shps->shapes.erase(res);
			//std::cout << "Removing shape " << shp->name << std::endl;
			delete shp;
			break;
		}
	}
}

// remove a group of shapes from the scene
void remove_shapes_from_scene(scene* scn, std::vector<shape*>& vecshp) {
	for(auto shp : vecshp){
		remove_shape_from_scene(scn, shp);
	}
}

//
void print_vector(std::vector<shape*>& vec) {
	for(shape* shp : vec){
		std::cout << shp->name << "   " << shp << std::endl;
	}
}

// get the instance that contains the given shape
auto get_instance_by_shape(scene* scn, shape* shp) -> instance* {
	instance* r = nullptr;
    for (auto instance : scn->instances){
		auto res = std::find(instance->shp->shapes.begin(), instance->shp->shapes.end(), shp);
        if(res != instance->shp->shapes.end()){
			if(r != nullptr) std::cout << "Aiuto" << std::endl;
			r = instance;
        }
    }
	return r;
}

// makes material from given kd and kd texture
material* make_material(const std::string& name, const vec3f& kd,
	const std::string& kd_txt, const vec3f& ks = { 0.04f, 0.04f, 0.04f },
	float rs = 0.01f) {

	material* mat = new material{ name };
	mat->kd = kd;
	mat->kd_txt = new texture{ name, kd_txt };
	mat->ks = ks;
	mat->rs = rs;
	return mat;
}

// add texture coordinates to mono-quad shapes.
void add_texcoord_to_quad_shape(shape* shp) {
	shp->texcoord.push_back(vec2f{ 0, 1 });
	shp->texcoord.push_back(vec2f{ 1, 1 });
	shp->texcoord.push_back(vec2f{ 1, 0 });
	shp->texcoord.push_back(vec2f{ 0, 0 });
}

// translate shapes
void translate(shape* shp, const vec3f t) {
	for (int i = 0; i < shp->pos.size(); i++) {
		shp->pos.at(i).x += t.x;
		shp->pos.at(i).y += t.y;
		shp->pos.at(i).z += t.z;
	}
}

// generates the map of the city, as a grid, with random streets width
void make_map(scene* scn) {
	//random map size
	rng_pcg32 rng = rng_pcg32{};
	seed_rng(rng, rng.state, rng.inc); //not so random, generate same random float at each execution, fix it!
	advance_rng(rng);
	float random_n = next_rand1f(rng, min_map_side, max_map_side);
	float random_m = next_rand1f(rng, min_map_side, max_map_side);
	//printf("n = %f\n", random_n);
	//printf("m = %f\n", random_m);
	//
	std::vector<shape*> zv = std::vector<shape*>();
	float x = -(random_n / 2.0f); //x-axis
	float z = -(random_m / 2.0f); //z-axis
	while (z < random_m / 2.0f) {
		float building_l = next_rand1f(rng, min_building_side, max_building_side);
		float street_w = next_rand1f(rng, min_street_width, max_street_width);
		shape* shp = new shape();
		shp->pos.push_back(vec3f{ x, 0, z });
		shp->pos.push_back(vec3f{ (random_n / 2.0f), 0, z });
		shp->pos.push_back(vec3f{ (random_n / 2.0f), 0, z + building_l });
		shp->pos.push_back(vec3f{ x, 0, z + building_l });
		shp->quads.push_back(vec4i{ 0, 1, 2, 3 });
		zv.push_back(shp);
		z += building_l + street_w;
	}
	//
	std::vector<shape*> xv = std::vector<shape*>();
	while (x < random_n / 2.0f) {
		float building_w = next_rand1f(rng, min_building_side, max_building_side);
		float street_w = next_rand1f(rng, min_street_width, max_street_width);
		for (shape* zshp : zv) {
			shape* shp = new shape();
			shp->quads.push_back(vec4i{ 0, 1, 2, 3 });
			shp->pos.push_back(vec3f{x, 0, zshp->pos.at(0).z});
			shp->pos.push_back(vec3f{x + building_w, 0, zshp->pos.at(1).z});
			shp->pos.push_back(vec3f{x + building_w, 0, zshp->pos.at(2).z});
			shp->pos.push_back(vec3f{x, 0, zshp->pos.at(3).z});
			xv.push_back(shp);
		}
		x += building_w + street_w;
	}
	//
	for (shape* shp : zv) {
		delete shp;
	}
	zv.clear();
	//
	for (shape* shp : xv) {
		add_instance(scn, "base", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, shp);
	}

}

// given a base and a height, creates the building
scene* extrude(scene* scn, instance* building, float height) {
	//initial shape is only a single quad, the base of the building
	shape* base = building->shp->shapes.at(0);
	if ( (base->quads.size() != 1) && (base->pos.size() != 4) ) {
		return nullptr;
	}
	//make building material
	//material* mat = make_material("building", vec3f{ 1.0, 1.0, 1.0 }, "texture.png");
	//scn->materials.push_back(mat);
	//scn->textures.push_back(mat->kd_txt);
	//make the entire building, creating the other 5 faces
	shape* roof = new shape{"roof"};
	for (int i = 0; i < base->pos.size(); i++) {
		vec3f vertex = vec3f{ base->pos.at(i).x, base->pos.at(i).y + height, base->pos.at(i).z }; //the x axis is for width the y axis is for height, the z axis is for length
		roof->pos.push_back(vertex);
	}
	roof->quads.push_back(vec4i{ 0, 1, 2, 3 });
	add_texcoord_to_quad_shape(roof);
	building->shp->shapes.push_back(roof);

	for (int j = 0; j < base->pos.size(); j++) {
		shape* facade = new shape{"facade"};
		facade->pos.push_back(vec3f{ base->pos.at(j).x, base->pos.at(j).y, base->pos.at(j).z });
		facade->pos.push_back(vec3f{ base->pos.at((j + 1) % 4).x, base->pos.at((j + 1) % 4).y, base->pos.at((j + 1) % 4).z });
		facade->pos.push_back(vec3f{ base->pos.at((j + 1) % 4).x, base->pos.at((j + 1) % 4).y + height, base->pos.at((j + 1) % 4).z });
		facade->pos.push_back(vec3f{ base->pos.at(j).x, base->pos.at(j).y + height, base->pos.at(j).z });
		facade->quads.push_back(vec4i{ 0, 1, 2, 3 });
		add_texcoord_to_quad_shape(facade);
		building->shp->shapes.push_back(facade);
	}

	building->name = "building";
	return scn;
}

// facade split on y-axis
std::vector<shape*> split_y(scene* scn, shape* shp, const std::vector<float> v, const std::vector<std::string> types) {
	//std::cout << "Splitting y axis of shape " << shp->name << std::endl;
	float check = 0;
	for (int i = 0; i < v.size(); i++) {
		check += v.at(i);
	}
	std::vector<shape*> newv = std::vector<shape*>();
	if (check != 1 || v.size() != types.size()) {
		//std::cout << check << "   " << v.size() << "   " << types.size() << std::endl;
		//return newv;
	}

	float x0 = shp->pos.at(0).x; //same x for v0 and v3
	float x1 = shp->pos.at(1).x; //same x for v1 and v2
	float z0 = shp->pos.at(0).z; //same z for v0 and v3
	float z1 = shp->pos.at(1).z; //same z for v1 and v2
	float y = shp->pos.at(0).y;  //same y for v0 and v1
	float h = shp->pos.at(2).y - shp->pos.at(1).y;//height

	for (int j = 0; j < v.size(); j++) {
		shape* nshp = new shape();

		nshp->name = types.at(j);
		nshp->pos.push_back(vec3f{ x0, y, z0 });
		nshp->pos.push_back(vec3f{ x1, y, z1 });
		nshp->pos.push_back(vec3f{ x1, y + v.at(j)*h, z1 });
		nshp->pos.push_back(vec3f{ x0, y + v.at(j)*h, z0 });
		add_texcoord_to_quad_shape(nshp);
		nshp->quads.push_back(vec4i{ 0, 1, 2, 3 });
		newv.push_back(nshp);
		y += v.at(j)*h;
		//std::cout << "Generating  shape " << nshp->name << std::endl;
	}
	return newv;
}

// facade repeat on y-axis
std::vector<shape*> repeat_y(scene* scn, shape* shp, int parts, const std::string& type) {
	if (parts < 1) {
		return std::vector<shape*>();
	}
	std::vector<float> v = std::vector<float>();
	std::vector<std::string> types = std::vector<std::string>();

	for (int i = 0; i < parts; i++) {
		v.push_back(1.0f / (float)parts);
		types.push_back(type);
	}
	return split_y(scn, shp, v, types);
}

// facade split on x-axis
std::vector<shape*> split_x(scene* scn, shape* shp, std::vector<float> v, const std::vector<std::string> types) {
	//std::cout << "Splitting x axis of shape " << shp->name << std::endl;
	float check = 0;
	for (int i = 0; i < v.size(); i++) {
		check += v.at(i);
	}
	std::vector<shape*> newv = std::vector<shape*>();
	if (check != 1) {
		printf("fail\n"); //
		return newv;
	}

	float y0 = shp->pos.at(0).y; //same y for v0 and v1
	float y1 = shp->pos.at(3).y; //same y for v2 and v3
	float x = shp->pos.at(0).x;  //same x for v0 and v3
	float z = shp->pos.at(0).z;  //same z for v0 and v3
	float w = shp->pos.at(1).x - shp->pos.at(0).x; //width
	float l = shp->pos.at(1).z - shp->pos.at(0).z; //length

	for (int j = 0; j < v.size(); j++) {
		shape* nshp = new shape();
		nshp->name = types.at(j);
		nshp->pos.push_back(vec3f{ x, y0, z });
		nshp->pos.push_back(vec3f{ x + v.at(j)*w, y0, z + v.at(j)*l });
		nshp->pos.push_back(vec3f{ x + v.at(j)*w, y1, z + v.at(j)*l });
		nshp->pos.push_back(vec3f{ x, y1, z });
		add_texcoord_to_quad_shape(nshp);
		nshp->quads.push_back(vec4i{ 0, 1, 2, 3 });
		newv.push_back(nshp);
		x += v.at(j)*w;
		z += v.at(j)*l;
		//std::cout << "Generating  shape " << nshp->name << std::endl;

	}
	return newv;
}

// facade repeat on x-axis
std::vector<shape*> repeat_x(scene* scn, shape* shp, int parts, const std::string& type) {
	if (parts < 1) {
		return std::vector<shape*>();
	}
	std::vector<float> v = std::vector<float>();
	std::vector<std::string> types = std::vector<std::string>();
	for (int i = 0; i < parts; i++) {
		v.push_back(1.0f / (float)parts);
		types.push_back(type);
	}
	return split_x(scn, shp, v, types);
}

//
uint32_t calculate_floors(uint32_t height) {
	uint32_t floor_h = height / 3.5;  // hardocded for now TODO, make chose from
									  // range, maybe using an approximation
	//std::cout << "Height: " << height << " Number of foors: " << floor_h << std::endl;
	return floor_h;
}

//
float calculate_floors_part(uint32_t height) {
	float part_f = 1.0f / (height / 3.5);
	return part_f;
}

//recursively divides a facade in subparts
std::vector<shape*> subdiv_facade(scene* scn, building_inst inst, shape* shp) {
	std::vector<shape*> to_add = std::vector<shape*>();
	if (shp->name == "facade") {
		float bottomfloor_p = calculate_floors_part(inst.info.h);//
		for (shape* nshp : split_y(scn, shp, std::vector<float>{ bottomfloor_p, 1.0f - bottomfloor_p }, std::vector<std::string>{ "bottomfloor", "topfloors" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}		
	}
	else if (shp->name == "topfloors") {
		for (shape* nshp : repeat_y(scn, shp, calculate_floors(inst.info.h - 1), "floor")) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}
	}
	else if (shp->name == "bottomfloor") {
		for (shape* nshp : split_x(scn, shp, std::vector<float>{ 0.4, 0.2, 0.4 }, std::vector<std::string>{ "tiles", "doortile", "tiles" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}
	}
	else if (shp->name == "doortile") {
		for (shape* nshp : split_x(scn, shp, std::vector<float>{ 0.1, 0.8, 0.1 }, std::vector<std::string>{ "vwall", "doorcol", "vwall" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			if(nshp->name == "doorcol") delete nshp;
		}
	}
	else if (shp->name == "doorcol") {
		for (shape* nshp : split_y(scn, shp, std::vector<float>{ 0.9, 0.1 }, std::vector<std::string>{ "door", "hwall" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
		}
	}
	else if (shp->name == "tiles") {
		for (shape* nshp : repeat_x(scn, shp, 2, "tile")) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}
	}
	else if (shp->name == "floor") {
		for (shape* nshp : repeat_x(scn, shp, 5, "tile")) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}
	}
	else if (shp->name == "tile") {
		for (shape* nshp : split_x(scn, shp, std::vector<float>{ 0.1, 0.8, 0.1 }, std::vector<std::string>{ "vwall", "windcol", "vwall" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			if(nshp->name == "windcol") delete nshp;
		}
	}
	else if (shp->name == "windcol") {
		for (shape* nshp : split_y(scn, shp, std::vector<float>{ 0.1, 0.8, 0.1 }, std::vector<std::string>{ "hwall", "window", "hwall" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
		}
	}
	else {
		to_add.push_back(shp);
	}
	return to_add;
}

static material* roof_tx = make_material("roof", vec3f{ 1.0f, 1.0f, 1.0f }, "roof.png");
static material* vwall_tx = make_material("vwall", vec3f{ 1.0f, 1.0f, 1.0f }, "vwall.png");
static material* hwall_tx = make_material("hwall", vec3f{ 1.0f, 1.0f, 1.0f }, "hwall.png");
static material* window_tx = make_material("window", vec3f{ 1.0f, 1.0f, 1.0f }, "window.png", vec3f{ 0.4f, 0.4f, 0.4f });
static material* door_tx = make_material("door", vec3f{ 1.0f, 1.0f, 1.0f }, "door.png");

// apply texture
void apply_material_and_texture(scene* scn, instance* inst) {
	for (shape* shp : inst->shp->shapes) {
		if (shp->name == "roof") {
			shp->mat = roof_tx;
		}
		if (shp->name == "vwall") {
			shp->mat = vwall_tx;
		}
		if (shp->name == "hwall") {
			shp->mat = hwall_tx;
		}
		else if (shp->name == "window") {
			shp->mat = window_tx;
		}
		else if (shp->name == "door") {
			shp->mat = door_tx;
		}
	}
}

//modified from model.cpp
scene* init_scene() {
	auto scn = new scene();
	//add floor
	material* mat = make_material( "terrain", { 0.2f, 0.2f, 0.2f }, "grid.png" );
	float p = max_map_side / 2.0f;
	shape* terrain = new shape{ "terrain" };
	terrain->mat = mat;
	terrain->pos = { { -p , 0, -p },{ p, 0, -p },{ p, 0, p },{ -p, 0, p } };
	terrain->norm = { { 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 } };
	terrain->quads.push_back(vec4i{ 0, 1, 2, 3 });
	add_texcoord_to_quad_shape(terrain);
	add_instance(scn, "terrain", identity_frame3f, terrain);
	scn->textures.push_back(terrain->mat->kd_txt);
	scn->materials.push_back(terrain->mat);

	//add light
	auto lmat = new material{ "light" };
	lmat->ke = { 100, 100, 100 };
	auto lshp = new shape{ "light" };
	lshp->mat = lmat;
	lshp->pos = { { 1.4f, 8, 6 },{ -1.4f, 8, 6 } };
	lshp->points = { 0, 1 };
	shape_group* lshpgrp = new shape_group{};
	lshpgrp->shapes.push_back(lshp);
	scn->shapes.push_back(lshpgrp);
	scn->materials.push_back(lmat);
	scn->instances.push_back(new instance{ "light", identity_frame3f, lshpgrp });

	// add camera
	auto cam = new camera{ "cam" };
	vec3f x = vec3f{ 0, 4, 10 };
	vec3f y = vec3f{ 0, 1, 0 };
	vec3f z = vec3f{ 0, 1, 0 };
	cam->frame = lookat_frame(x,y,z);
	cam->yfov= 15 * pif / 180.f;
	cam->aspect = 16.0f / 9.0f;
	cam->aperture = 0;
	cam->focus = length(vec3f{ 0, 4, 10 } -vec3f{ 0, 1, 0 });
	scn->cameras.push_back(cam);
	return scn;
}


int main(int argc, char** argv ) {
	//parsing
	auto parser =
		make_parser(argc, argv, "model", "creates simple scenes");
	auto sceneout = parse_opt(
		parser, "--output", "-o", "output scene", "out.obj"s);

	//printf("creating scene %s\n", type.c_str());

	//make scene
	scene* scn = init_scene();

	//generate grid map
	printf("generating map\n");
	make_map(scn);

	//make buildings from basements
	printf("extruding buildings\n");
	int count_extruded = 1;//
	std::uniform_int_distribution<uint32_t> h_range(skyscraper_constants.h_range.first, skyscraper_constants.h_range.second);
	auto height = bind_random_distribution(h_range);
	std::vector<shape*> base_to_remove = std::vector<shape*>();
	for (instance* inst : scn->instances) {
		if (inst->name == "base") {
			uint32_t roll = height();
			extrude(scn, inst, roll);
			//printf("extruded %d\n", count_extruded);//
			count_extruded++;//
			base_to_remove.push_back(inst->shp->shapes.at(0));
		}
	}
	remove_shapes_from_scene(scn, base_to_remove);

	//subdivide building facade
	printf("subdividing facades\n");
	for (instance* inst : scn->instances) {
		std::vector<shape*> to_add = std::vector<shape*>();
		std::vector<shape*> to_remove = std::vector<shape*>();
		for (shape* shp : inst->shp->shapes) {
			if (shp->name == "facade") {
				building_info bin = { skyscraper,0,0,0,0,0,0,true }; //TODO reuse previous generated building
				bin.h = shp->pos.at(2).y - shp->pos.at(1).y;
				building_inst bv = { inst, bin };
				auto res = subdiv_facade(scn, bv, shp);
				std::copy(res.begin(), res.end(), back_inserter(to_add));
				to_remove.push_back(shp);
			}
		}
		remove_shapes_from_scene(scn, to_remove);
		for (auto shp : to_add) {
			inst->shp->shapes.push_back(shp);
		}
		to_remove.clear();
		to_add.clear();
	}

	//adding material to the scene
	scn->materials.push_back(roof_tx);
	scn->textures.push_back(roof_tx->kd_txt);
	scn->materials.push_back(vwall_tx);
	scn->textures.push_back(vwall_tx->kd_txt);
	scn->materials.push_back(hwall_tx);
	scn->textures.push_back(hwall_tx->kd_txt);
	scn->materials.push_back(window_tx);
	scn->textures.push_back(window_tx->kd_txt);
	scn->materials.push_back(door_tx);
	scn->textures.push_back(door_tx->kd_txt);

	//material and texture application
	printf("applying material and textures\n");
	int count_textured = 1;//
	for (instance* inst : scn->instances) {
		if (inst->name == "building") {
			apply_material_and_texture(scn, inst);
			//printf("textured %d\n", count_textured);//
			//count_textured++;//
		}
	}

	//save
	save_options sopt = save_options();
	printf("saving scene %s\n", sceneout.c_str());
	save_scene(sceneout, scn, sopt );
	delete scn;
	return 0;
}