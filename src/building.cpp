#include <vector>
#include "../yocto/yocto_gl.h"

using namespace ygl;

//note : obj files, mtl files and png files for textures must be all in the same folder!

enum building_type { residential, skyscraper, office, tower, house };

enum subdiv_axis { x, y };

enum element_type {};

struct type_info {
	std::map<building_type, std::string> type_to_str = std::map<building_type, std::string>();
	building_type type;
	float min_h, max_h;
	float max_l, min_l;
	float max_w, min_w;
	float max_w_l_ratio, min_w_l_ratio;
	float max_floor_h, min_floor_h;
	float max_wind_w, min_wind_w;
	float max_wind_h, min_wind_h;

	type_info(building_type type) {
		type_to_str.insert(std::pair<building_type, std::string>(residential, "residential"));
		type_to_str.insert(std::pair<building_type, std::string>(skyscraper, "skyscraper"));
		type_to_str.insert(std::pair<building_type, std::string>(office, "office"));
		type_to_str.insert(std::pair<building_type, std::string>(tower, "tower"));
		type_to_str.insert(std::pair<building_type, std::string>(house, "house"));
	}

	~type_info() {

	}

};

struct building_info {
	building_type type;
	float h;
	float l;
	float w;
	float w_l_ratio;
	float floor_h;
	float wind_w;
	float wind_h;
	bool flat_roof;
	std::map<element_type, std::pair<std::vector<element_type>, subdiv_axis>> subdiv_rules;

	building_info() {
		
	}


	~building_info() {

	}
};

//
struct building {
	instance* building;
	building_info info;
};

//add instance
void add_instance(scene* scn, const std::string& name, const frame3f& f,
	shape* shp) {
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
	//TODO add material to scene must be in a function to add a shape to an instance.
}

//Add the shape to the instance
void add_shape_in_instance(instance* inst , shape* shp){
	inst->shp->shapes.push_back(shp);
}


void remove_shape_from_scene(scene* scn, shape* shp){
<<<<<<< HEAD
	if (shp->mat != nullptr) {
		auto res1 = std::find(scn->materials.begin(), scn->materials.end(), shp->mat);
		if (res1 != scn->materials.end()) {
			scn->materials.erase(res1);
			delete shp->mat->kd_txt;
		}
		auto res2 = std::find(scn->textures.begin(), scn->textures.end(), shp->mat->kd_txt);
		if (res2 != scn->textures.end()) {
			scn->textures.erase(res2);
=======
	if (shp->mat != nullptr){
		auto res2 = std::find(scn->textures.begin(), scn->textures.end(), shp->mat->kd_txt);
		if(res2 != scn->textures.end()){
			scn->textures.erase(res2);
			delete shp->mat->kd_txt;
		}
		auto res1 = std::find(scn->materials.begin(), scn->materials.end(), shp->mat);
		if(res1 != scn->materials.end()){
			scn->materials.erase(res1);
>>>>>>> f6bd3b7713d1763892757de3c0aa6ffb11f1dede
			delete shp->mat;
		}
	}
	for(auto shps : scn->shapes){
		auto res = std::find(shps->shapes.begin(), shps->shapes.end(), shp);
		if(res != shps->shapes.end()){
			shps->shapes.erase(res);
<<<<<<< HEAD
//			std::cout << "Removing shape " << shp->name << std::endl;
=======
			std::cout << "Removing shape " << shp->name << std::endl;
>>>>>>> f6bd3b7713d1763892757de3c0aa6ffb11f1dede
			delete shp;
			break;
		}
	}
<<<<<<< HEAD
=======

>>>>>>> f6bd3b7713d1763892757de3c0aa6ffb11f1dede
}


void remove_shapes_from_scene(scene* scn, std::vector<shape*>& vecshp){
	for(auto shp : vecshp){
		remove_shape_from_scene(scn, shp);
	}
}


void print_vector(std::vector<shape*>& vec){
	for(shape* shp : vec){
		std::cout << shp->name << "   "    << shp << std::endl;
	}
}


auto get_instance_by_shape(scene* scn, shape* shp) -> instance*{
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
/*
void remove_instance(scene* scn, shape* shp) {
	//remove shape and instance
	//material and texture?
    
    std::vector<int> index = std::vector<int>();
    size_t currentIndex = 0;
    
    for (auto shp_group : scn->shapes){
	//assuming every shape_group containst one shape / every shape is stored in a different group
        if(shp_group->shapes.at(0) == shp){
            index.push_back(currentIndex); 
            std::cout << "Removing shape of index " << currentIndex << " Shape "<< shp->name << std::endl;
        }
        currentIndex++;
    }
    for (auto ind = index.rbegin() ; ind != index.rend(); ind++){
        auto curr_iter = scn->shapes.begin();
        std::advance(curr_iter, *ind);
        scn->shapes.erase(curr_iter);
    }
    
    index.clear();
    currentIndex = 0;
    
    for (auto inst : scn->instances){
	//assuming every shape_group containst one shape / every shape is stored in a different group
        if(inst->shp->shapes.at(0) == shp){
            index.push_back(currentIndex); 
            std::cout << "Removing index " << currentIndex << std::endl;
        }
        currentIndex++;
    }
    for (auto ind = index.rbegin() ; ind != index.rend(); ind++){
        auto curr_iter = scn->instances.begin();
        std::advance(curr_iter, *ind);
        scn->instances.erase(curr_iter);
    }
}*/

//makes material from given kd and kd texture
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

//add texture coordinates to mono-quad shapes.
void add_texcoord_to_quad_shape(shape* shp) {
	shp->texcoord.push_back(vec2f{ 0, 1 });
	shp->texcoord.push_back(vec2f{ 1, 1 });
	shp->texcoord.push_back(vec2f{ 1, 0 });
	shp->texcoord.push_back(vec2f{ 0, 0 });
}

//given a base and a height, creates the building
scene* extrude(scene* scn, instance* building, float height) {
	//initial shape is only a single quad, the base of the building
	shape* base = building->shp->shapes.at(0);
	if ( (base->quads.size() != 1) && (base->pos.size() != 4) ) {
		return nullptr;
	}
	//make building material
	//material* mat = make_material("building", vec3f{ 1.0, 1.0, 1.0 }, "texture.png");
//	scn->materials.push_back(mat);
//	scn->textures.push_back(mat->kd_txt);
	//make the entire building, creating the other 5 faces
	shape* roof = new shape{"roof"};
	for (int i = 0; i < base->pos.size(); i++) {
		vec3f vertex = vec3f{ base->pos.at(i).x, base->pos.at(i).y + height, base->pos.at(i).z }; //the x axis is for lenght, the y axis is for height, the z axis is for depth
		roof->pos.push_back(vertex);
	}
	roof->quads.push_back(vec4i{ 0, 1, 2, 3 });
	//roof->mat = mat;
	add_texcoord_to_quad_shape(roof);
	building->shp->shapes.push_back(roof);

	for (int j = 0; j < base->pos.size(); j++) {
		shape* facade = new shape{"facade"};
		facade->pos.push_back(vec3f{ base->pos.at(j).x, base->pos.at(j).y, base->pos.at(j).z });
		facade->pos.push_back(vec3f{ base->pos.at((j + 1) % 4).x, base->pos.at((j + 1) % 4).y, base->pos.at((j + 1) % 4).z });
		facade->pos.push_back(vec3f{ base->pos.at((j + 1) % 4).x, base->pos.at((j + 1) % 4).y + height, base->pos.at((j + 1) % 4).z });
		facade->pos.push_back(vec3f{ base->pos.at(j).x, base->pos.at(j).y + height, base->pos.at(j).z });
		facade->quads.push_back(vec4i{ 0, 1, 2, 3 });
		//facade->mat = mat;
		add_texcoord_to_quad_shape(facade);
		building->shp->shapes.push_back(facade);
	}

	building->name = "building";
	return scn;
}

//translate shapes
void translate(shape* shp, const vec3f t) {
	for (int i = 0; i < shp->pos.size(); i++) {
		shp->pos.at(i).x += t.x;
		shp->pos.at(i).y += t.y;
		shp->pos.at(i).z += t.z;
	}
}

//facade split on y-axis
std::vector<shape*> split_y(scene* scn, shape* shp, const std::vector<float> v, const std::vector<std::string> types) {
//	std::cout << "Splitting y axis of shape " << shp->name << std::endl;
	float check = 0;
	for (int i = 0; i < v.size(); i++) {
		check += v.at(i);
	}
	std::vector<shape*> newv = std::vector<shape*>();
	if (check != 1 || v.size() != types.size()) {
//		printf("unconsistent data\n"); //
		return newv;
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
//		std::cout << "Generating  shape " << nshp->name << std::endl;
	}
	return newv;
}

//facade repeat on y-axis
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

//facade split on x-axis
std::vector<shape*> split_x(scene* scn, shape* shp, std::vector<float> v, const std::vector<std::string> types) {
//	std::cout << "Splitting x axis of shape " << shp->name << std::endl;
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
//		std::cout << "Generating  shape " << nshp->name << std::endl;

	}
	return newv;
}

//facade repeat on x-axis
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

//recursively divides a facade in subparts
std::vector<shape*> subdiv_facade(scene* scn, instance* inst, shape* shp) {
	std::vector<shape*> to_add = std::vector<shape*>();
	if (shp->name == "facade") {
		for (shape* nshp : split_y(scn, shp, std::vector<float>{ 0.1, 0.9 }, std::vector<std::string>{ "bottomfloor", "topfloors" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}		
	}
	else if (shp->name == "topfloors") {
		for (shape* nshp : repeat_y(scn, shp, 9, "floor")) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}
		delete shp;
	}
	else if (shp->name == "bottomfloor") {
		for (shape* nshp : split_x(scn, shp, std::vector<float>{ 0.4, 0.2, 0.4 }, std::vector<std::string>{ "tiles", "doortile", "tiles" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}
		delete shp;
	}
	else if (shp->name == "doortile") {
		for (shape* nshp : split_x(scn, shp, std::vector<float>{ 0.1, 0.8, 0.1 }, std::vector<std::string>{ "vwall", "doorcol", "vwall" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			if(nshp->name == "doorcol") delete nshp;
		}
		delete shp;
	}
	else if (shp->name == "doorcol") {
		for (shape* nshp : split_y(scn, shp, std::vector<float>{ 0.9, 0.1 }, std::vector<std::string>{ "door", "hwall" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
		}
		delete shp;
	}
	else if (shp->name == "tiles") {
		for (shape* nshp : repeat_x(scn, shp, 2, "tile")) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}
		delete shp;
	}
	else if (shp->name == "floor") {
		for (shape* nshp : repeat_x(scn, shp, 5, "tile")) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			delete nshp;
		}
		delete shp;
	}
	else if (shp->name == "tile") {
		for (shape* nshp : split_x(scn, shp, std::vector<float>{ 0.1, 0.8, 0.1 }, std::vector<std::string>{ "vwall", "windcol", "vwall" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
			if(nshp->name == "windcol") delete nshp;
		}
		delete shp;
	}
	else if (shp->name == "windcol") {
		for (shape* nshp : split_y(scn, shp, std::vector<float>{ 0.1, 0.8, 0.1 }, std::vector<std::string>{ "hwall", "window", "hwall" })) {
			auto res = subdiv_facade(scn, inst, nshp);
			std::copy(res.begin(), res.end(), back_inserter(to_add));
		}
		delete shp;
	}
	else {
		to_add.push_back(shp);
	}
	return to_add;
}

//apply texture
void apply_material_and_texture(scene* scn, instance* inst) {
	for (shape* shp : inst->shp->shapes) {
		if (shp->name == "roof") {
			shp->mat = make_material("roof", vec3f{ 1.0f, 1.0f, 1.0f }, "roof.png");
			shp->subdivision = 5;												//
			tesselate_shape(shp, true, false, false, false);					//
			shp->mat->disp_txt = new texture{ "roof", "displace_roof_test" };	//test
		}
		if (shp->name == "vwall") {
			shp->mat = make_material("vwall", vec3f{ 1.0f, 1.0f, 1.0f }, "vwall.png");
		}
		if (shp->name == "hwall") {
			shp->mat = make_material("hwall", vec3f{ 1.0f, 1.0f, 1.0f }, "hwall.png");
		}
		else if (shp->name == "window") {
			shp->mat = make_material("window", vec3f{ 1.0f, 1.0f, 1.0f }, "window.png", vec3f{ 0.4f, 0.4f, 0.4f });
		}
		else if (shp->name == "door") {
			shp->mat = make_material("door", vec3f{ 1.0f, 1.0f, 1.0f }, "door.png");
		}
		scn->textures.push_back(shp->mat->kd_txt);
		scn->materials.push_back(shp->mat);
	}
}

//modified from model.cpp
scene* init_scene() {
	auto scn = new scene();
	//add floor
	material* mat = make_material( "floor", { 0.2f, 0.2f, 0.2f }, "grid.png" );

	shape* shp = new shape{ "floor" };
	shp->mat = mat;
	shp->pos = { { -20, 0, -20 },{ 20, 0, -20 },{ 20, 0, 20 },{ -20, 0, 20 } };
	shp->norm = { { 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 } };
	shp->texcoord = { { -10, -10 },{ 10, -10 },{ 10, 10 },{ -10, 10 } };
	shp->triangles = { { 0, 1, 2 },{ 0, 2, 3 } };

	add_instance(scn, "floor", identity_frame3f, shp);
	scn->textures.push_back(shp->mat->kd_txt);
	scn->materials.push_back(shp->mat);

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
	scn->instances.push_back(
		new instance{ "light", identity_frame3f, lshpgrp });
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

	//make dummy material
	//material* mat = make_material("building", { 1.0f, 1.0f, 1.0f }, "colored.png");

	//make base
	shape* base = new shape{ "base" };
	base->pos.push_back(vec3f{ -5,  0, -5 });
	base->pos.push_back(vec3f{ -5,  0,  5 });
	base->pos.push_back(vec3f{  5,  0,  5 });
	base->pos.push_back(vec3f{  5,  0, -5 });
	base->quads.push_back(vec4i{ 0, 1, 2, 3 });
	//base->mat = mat;

	//make base 2
	shape* base2 = new shape{ "base" };
	base2->pos.push_back(vec3f{  3,  0, -1 });
	base2->pos.push_back(vec3f{  3,  0,  1 });
	base2->pos.push_back(vec3f{  9,  0,  3 });
	base2->pos.push_back(vec3f{  9,  0, -3 });
	base2->quads.push_back(vec4i{ 0, 1, 2, 3 });
	//base2->mat = mat;

	//make scene
	scene* scn = init_scene();
	add_instance(scn, "base",  frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, base);
	//add_instance(scn, "base", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, base2);

	//make buildings from basement
	instance* building = get_instance_by_shape(scn, base);
	//instance* building2 = get_instance_by_shape(scn, base2);
	extrude(scn, building, 30.0f);
	//extrude(scn, building2, 10.0f);
	remove_shape_from_scene(scn, base);
<<<<<<< HEAD
	remove_shape_from_scene(scn, base2);
	delete base2;

=======
//	remove_shape_from_scene(scn, base2);
	delete base2; //Temp
>>>>>>> f6bd3b7713d1763892757de3c0aa6ffb11f1dede
	//subdivide building facade
	for (instance* inst : scn->instances) {
		std::vector<shape*> to_add = std::vector<shape*>();
		std::vector<shape*> to_remove = std::vector<shape*>();
		for (shape* shp : inst->shp->shapes) {
			if (shp->name == "facade") {
				auto res = subdiv_facade(scn, inst, shp);
				std::copy(res.begin(), res.end(), back_inserter(to_add));
				to_remove.push_back(shp);
			}
		}
		remove_shapes_from_scene(scn, to_remove);
		for(auto shp : to_add) {
			inst->shp->shapes.push_back(shp);
		}
	}
	
	//
	for (instance* inst : scn->instances) {
		if (inst->name == "building") {
			apply_material_and_texture(scn, inst);
		}
	}

	//save
	save_options sopt = save_options();
	printf("saving scene %s\n", sceneout.c_str());
	save_scene(sceneout, scn, sopt );
	delete scn;
	return 0;
}