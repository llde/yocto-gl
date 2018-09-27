#include <vector>
#include "../yocto/yocto_gl.h"
//#include "../yocto/yocto_gltf.h"

using namespace ygl;


//note : obj files, mtl files and png files for textures must be all in the same folder!

//modified from model.cpp
void add_instance(scene* scn, const std::string& name, const frame3f& f,
	shape* shp, material* mat) {
	if (!shp || !mat) return;

	shape_group* shpgrp = new shape_group{};
	shpgrp->shapes.push_back(shp);

	instance* inst = new instance{ name, f, shpgrp };
	if (std::find(scn->instances.begin(), scn->instances.end(), inst) == scn->instances.end()) {
		scn->instances.push_back(inst);
	}
	if (std::find(scn->shapes.begin(), scn->shapes.end(), shpgrp) == scn->shapes.end()) {
		scn->shapes.push_back(shpgrp);
	}
	if (std::find(scn->materials.begin(), scn->materials.end(), mat) == scn->materials.end()) {
		scn->materials.push_back(mat);
	}
	if (std::find(scn->textures.begin(), scn->textures.end(), mat->kd_txt) == scn->textures.end()) { //add texture check
		scn->textures.push_back(mat->kd_txt);
	}
}
/**
 * Add the shape to the instance
*/
void add_shape_in_instance(instance* inst , shape* shp){
	inst->shp->shapes.push_back(shp);
}
/*
void remove_shape_from_instance(instance* inst, shape* shp){
	int idx = -1;
	int i = 0;
	for(auto shps : inst->shp->shapes){
		if(shps == shp){
			idx = i;
			break;
		}
		i++;
	}
	if(idx == -1) {
		std::cout << "No shape in the specified instance " << std::endl; 
		return;
	}
	auto ret = inst->shp->shapes.begin();
	std::advance(ret, idx);
    std::cout << "Removing shape of index " << idx << " Shape "<< shp->name << std::endl;
	inst->shp->shapes.erase(ret);
}*/

void remove_shape_from_scene(scene* scn, shape* shp){
	for(auto shps : scn->shapes){
		auto res = std::find(shps->shapes.begin(), shps->shapes.end(), shp);
		if(res != shps->shapes.end()){
			shps->shapes.erase(res);
			std::cout << "Removing shape " << shp->name << std::endl;
			break;
		}
	}
	auto res1 = std::find(scn->materials.begin(), scn->materials.end(), shp->mat);
	if(res1 != scn->materials.end()){
		scn->materials.erase(res1);
	}
	auto res2 = std::find(scn->textures.begin(), scn->textures.end(), shp->mat->kd_txt);
	if(res2 != scn->textures.end()){
		scn->textures.erase(res2);
	}

}


auto get_instance_by_shape(scene* scn ,shape* shp) -> instance*{
    for (auto instance : scn->instances){
		auto res = std::find(instance->shp->shapes.begin(), instance->shp->shapes.end(), shp);
        if(res != instance->shp->shapes.end()){
			return instance;
        }
    }
	return nullptr;
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

//modified from model.cpp
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

scene* extrude(scene* scn, shape* base, float height) {
	//initial shape is only a single quad, the base of the building
	if ( (base->quads.size() != 1) && (base->pos.size() != 4) ) {
		return nullptr;
	}
	//make building material
	material* mat = make_material("building", vec3f{ 1.0, 1.0, 1.0 }, "texture.png");
	//make the entire building, creating the other 5 faces
	shape* roof = new shape{"roof"};
	for (int i = 0; i < base->pos.size(); i++) {
		vec3f vertex = vec3f{ base->pos.at(i).x, base->pos.at(i).y + height, base->pos.at(i).z }; //the x axis is for lenght, the y axis is for height, the z axis is for depth
		roof->pos.push_back(vertex);
	}
	roof->quads.push_back(vec4i{ 0, 1, 2, 3 });
	roof->mat = mat;
	roof->texcoord.push_back(vec2f{ 0, 0 });
	roof->texcoord.push_back(vec2f{ 0, 1 });
	roof->texcoord.push_back(vec2f{ 1, 1 });
	roof->texcoord.push_back(vec2f{ 1, 0 });
	add_instance(scn, "roof", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, roof, mat);

	for (int j = 0; j < base->pos.size(); j++) {
		shape* facade = new shape{"gfacade"};
		facade->pos.push_back(vec3f{ base->pos.at(j).x, base->pos.at(j).y, base->pos.at(j).z });
		facade->pos.push_back(vec3f{ base->pos.at((j + 1) % 4).x, base->pos.at((j + 1) % 4).y, base->pos.at((j + 1) % 4).z });
		facade->pos.push_back(vec3f{ base->pos.at((j + 1) % 4).x, base->pos.at((j + 1) % 4).y + height, base->pos.at((j + 1) % 4).z });
		facade->pos.push_back(vec3f{ base->pos.at(j).x, base->pos.at(j).y + height, base->pos.at(j).z });
		facade->quads.push_back(vec4i{ 0, 1, 2, 3 });
		facade->mat = mat;
		facade->texcoord.push_back(vec2f{ 0, 0 });
		facade->texcoord.push_back(vec2f{ 0, 1 });
		facade->texcoord.push_back(vec2f{ 1, 1 });
		facade->texcoord.push_back(vec2f{ 1, 0 });
		add_instance(scn, "facade", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, facade, mat);
	}

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
	std::cout << "Splitting y axis of shape " << shp->name << std::endl;
	float check = 0;
	for (int i = 0; i < v.size(); i++) {
		check += v.at(i);
	}
	std::vector<shape*> app = std::vector<shape*>();
	if (check != 1 || v.size() != types.size()) {
		printf("unconsistent data\n"); //
		return app;
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
		nshp->texcoord.push_back(vec2f{ 0, 0 });
		nshp->texcoord.push_back(vec2f{ 0, 1 });
		nshp->texcoord.push_back(vec2f{ 1, 1 });
		nshp->texcoord.push_back(vec2f{ 1, 0 });
		nshp->quads.push_back(vec4i{ 0, 1, 2, 3 });
		material* mat = new material{ types.at(j) };
		mat->kd = vec3f{ 1.0f, 1.0f, 1.0f }; //this is only for test
		mat->kd_txt = new texture{ types.at(j), "colored.png" };
		//note : a material without texture triggers a segmentation fault because add_instance pushes back a nullptr in a vector
		nshp->mat = mat;
		app.push_back(nshp);
//		instance* inst = get_instance_by_shape(scn, shp);
	//	add_instance(scn, types.at(j), frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, nshp, mat);
//		add_shape_in_instance(inst, nshp);
//		scn->materials.push_back(mat);
//		scn->textures.push_back(mat->kd_txt); //TODO add this to the helper functions or reuse existing materials/texture
		
		y += v.at(j)*h;
		std::cout << "Generating  shape " << nshp->name << std::endl;
	}
//	remove_shape_from_instance(get_instance_by_shape(scn, shp), shp);

//	remove_instance(scn, shp);
	return app;
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
	std::cout << "Splitting x axis of shape " << shp->name << std::endl;
	float check = 0;
	for (int i = 0; i < v.size(); i++) {
		check += v.at(i);
	}
	std::vector<shape*> app = std::vector<shape*>();
	if (check != 1) {
		printf("fail\n"); //
		return app;
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
		nshp->texcoord.push_back(vec2f{ 0, 0 });
		nshp->texcoord.push_back(vec2f{ 0, 1 });
		nshp->texcoord.push_back(vec2f{ 1, 1 });
		nshp->texcoord.push_back(vec2f{ 1, 0 });
		nshp->quads.push_back(vec4i{ 0, 1, 2, 3 });
		material* mat = new material{ types.at(j) };
		mat->kd = vec3f{ 1.0f, 1.0f, 1.0f }; //this is only for test
		mat->kd_txt = new texture{ types.at(j), "colored.png" };
		//note : a material without texture triggers a segmentation fault because add_instance pushes back a nullptr in a vector
		nshp->mat = mat;
		app.push_back(nshp);
//		instance* inst = get_instance_by_shape(scn, shp);
		//add_instance(scn, types.at(j), frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, nshp, mat);
//		add_shape_in_instance(inst, nshp);
//		scn->materials.push_back(mat);
//		scn->textures.push_back(mat->kd_txt); //TODO add this to the helper functions or reuse existing materials/texture
		x += v.at(j)*w;
		z += v.at(j)*l;
		std::cout << "Generating  shape " << nshp->name << std::endl;

	}
//	remove_shape_from_instance(get_instance_by_shape(scn, shp), shp);
	//TODO remove or reuse shp textures;
	//remove_instance(scn, shp);
	return app;
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

//bugged. not used for now.
scene* subdiv_facade(scene* scn) {
	for (instance* inst : scn->instances) {
		if (inst->name == "facade") {
			repeat_y(scn, inst->shp->shapes.at(0), 4, "floors");
			subdiv_facade(scn);
		}
		else if (inst->name == "floors") {
			repeat_x(scn, inst->shp->shapes.at(0), 4, "tile");
			subdiv_facade(scn);
		}
		else if (inst->name == "tile") {
			split_x(scn, inst->shp->shapes.at(0), std::vector<float>{ 0.1, 0.8, 0.1 }, std::vector<std::string>{ "vwall", "windcol", "vwall" });
			subdiv_facade(scn);
		}
		else if (inst->name == "windcol") {
			split_y(scn, inst->shp->shapes.at(0), std::vector<float>{ 0.1, 0.8, 0.1 }, std::vector<std::string>{ "hwall", "window", "hwall" });
			subdiv_facade(scn);
		}
		else if (inst->name == "vwall") {
			
		}
		else if (inst->name == "hwall") {

		}
		else if (inst->name == "window") {

		}
	}
	return scn;
}

//modified from model.cpp
scene* init_scene() {
	auto scn = new scene();
	// add floor
	material* mat = make_material( "floor", { 0.2f, 0.2f, 0.2f }, "grid.png" );

	shape* shp = new shape{ "floor" };
	shp->mat = mat;
	shp->pos = { { -20, 0, -20 },{ 20, 0, -20 },{ 20, 0, 20 },{ -20, 0, 20 } };
	shp->norm = { { 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 } };
	shp->texcoord = { { -10, -10 },{ 10, -10 },{ 10, 10 },{ -10, 10 } };
	shp->triangles = { { 0, 1, 2 },{ 0, 2, 3 } };

	add_instance(scn, "floor", identity_frame3f, shp, mat);

	// add light
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

	//make material
	material* mat = make_material("building", { 1.0f, 1.0f, 1.0f }, "colored.png");

	//make base
	shape* base = new shape();
	base->name = "building1";
	base->pos.push_back(vec3f{ -1,  0, -1 });
	base->pos.push_back(vec3f{ -1,  0,  1 });
	base->pos.push_back(vec3f{  1,  0,  1 });
	base->pos.push_back(vec3f{  1,  0, -1 });
	base->quads.push_back(vec4i{ 0, 1, 2, 3 });
	base->mat = mat;

	//make base 2
	shape* base2 = new shape();
	base->name = "building2";
	base2->pos.push_back(vec3f{  3,  0, -1 });
	base2->pos.push_back(vec3f{  3,  0,  1 });
	base2->pos.push_back(vec3f{  9,  0,  3 });
	base2->pos.push_back(vec3f{  9,  0, -3 });
	base2->quads.push_back(vec4i{ 0, 1, 2, 3 });
	base2->mat = mat;

	//translate base
	translate(base, vec3f{ -1,  0, -3 });

	//make a single vertical facade
	shape* facade = new shape();
	facade->name = "facade";
	facade->pos.push_back(vec3f{ -1,  0,  0 });
	facade->pos.push_back(vec3f{  1,  0,  0 });
	facade->pos.push_back(vec3f{  1,  2,  0 });
	facade->pos.push_back(vec3f{ -1,  2,  0 });
	facade->quads.push_back({ 0, 1, 2, 3 });
	facade->mat = mat;

	//make scene
	scene* scn = init_scene();
	add_instance(scn, "base",  frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, base, mat);
	add_instance(scn, "base", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, base2, mat);
	add_instance(scn, "facade", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, facade, mat);

	//make buildings from basement
	extrude(scn, base, 4.0f);
	extrude(scn, base2, 10.0f); //extrude call introduces a bug in the facade split. WTF?

	//split facade test
	std::vector<shape*> ss = std::vector<shape*>();
	instance* inst = get_instance_by_shape(scn, facade);
	for(shape* shpe : repeat_y(scn, facade, 3, "floors")){			
		ss.push_back(shpe);
		scn->textures.push_back(shpe->mat->kd_txt);
		scn->materials.push_back(shpe->mat);
		std::cout << "What I'm doing with my life" << std::endl;
	}
	
	inst->shp->shapes = ss;
	remove_shape_from_scene(scn, facade);
	//split floors test
	for (instance* inst : scn->instances) {
		std::vector<shape*> appoggio = std::vector<shape*>();
		if(inst->shp == nullptr) continue;
		for(shape* shp : inst->shp->shapes){
			std::cout << "Shape name " << shp->name << std::endl;
			if(shp->name == "floors") {
				for(shape* shpe : repeat_x(scn, shp , 3, "tile")){
					std::cout << "Processing shape " << shpe->name << std::endl;
					appoggio.push_back(shpe);
					scn->textures.push_back(shpe->mat->kd_txt);
					scn->materials.push_back(shpe->mat);
				}
				remove_shape_from_scene(scn, shp);
			}
			else  appoggio.push_back(shp);
		}
		inst->shp->shapes = appoggio; 
	}
	
	//subdiv_facade(scn);

	//save
	save_options sopt = save_options();
	printf("saving scene %s\n", sceneout.c_str());
	save_scene(sceneout, scn, sopt );
	delete scn;
	return 0;
}

