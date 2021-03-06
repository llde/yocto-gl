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

void remove_instance(scene* scn, shape* shp) {
	//remove shape and instance
	//material and texture?
    
    std::vector<int> index = std::vector<int>();
    size_t currentIndex = 0;
    
    for (auto shp_group : scn->shapes){
	//assuming every shape_group containst one shape / every shape is stored in a different group
        if(shp_group->shapes.at(0) == shp){
            index.push_back(currentIndex); 
            std::cout << "Removing index " << currentIndex << std::endl;
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
}

shape* make_building(shape* base, float height) {
	//initial shape is only a single quad, the base of the building
	if ( (base->quads.size() != 1) && (base->pos.size() != 4) ) {
		return nullptr;
	}
	//make the entire building, creating the other 5 faces
	std::vector<vec3f> roof_vertex = std::vector<vec3f>();
	for (int i = 0; i < base->pos.size(); i++) {
		vec3f vertex = vec3f{ base->pos.at(i).x, base->pos.at(i).y + height, base->pos.at(i).z }; //the x axis is for lenght, the y axis is for height, the z axis is for depth
		roof_vertex.push_back(vertex);
	}
	for (vec3f v : roof_vertex) {
		base->pos.push_back(v);
	}
	for (int j = 0; j < 4; j++) {
		vec4i side = vec4i{ j, (j+1)%4, (j+1)%4+4, j+4 };
		base->quads.push_back(side);
	}
	base->quads.push_back(vec4i{ 4, 5, 6, 7 });
	return base;
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
scene* split_y(scene* scn, shape* shp, std::vector<float> v, const std::string& type) {
	printf("check\n"); //
	float check = 0;
	for (int i = 0; i < v.size(); i++) {
		check += v.at(i);
	}
	if (check != 1) {
		printf("fail\n"); //
		return scn;
	}

	float x0 = shp->pos.at(0).x; //same x for v0 and v3
	float x1 = shp->pos.at(1).x; //same x for v1 and v2
	float z0 = shp->pos.at(0).z; //same z for v0 and v3
	float z1 = shp->pos.at(1).z; //same z for v1 and v2
	float y = shp->pos.at(0).y;  //same y for v0 and v1
	float h = shp->pos.at(2).y - shp->pos.at(1).y;//height

	material* mat = new material{ type };
	mat->kd = vec3f{ 1.0f, 1.0f, 1.0f }; //this is only for test
	mat->kd_txt = new texture{ type, "colored.png" };
	//note : a material without texture triggers a segmentation fault because add_instance pushes back a nullptr in a vector

	for (int j = 0; j < v.size(); j++) {
		shape* nshp = new shape();
		nshp->pos.push_back(vec3f{ x0, y, z0 });
		nshp->pos.push_back(vec3f{ x1, y, z1 });
		nshp->pos.push_back(vec3f{ x1, y + v.at(j)*h, z1 });
		nshp->pos.push_back(vec3f{ x0, y + v.at(j)*h, z0 });
		nshp->texcoord.push_back(vec2f{ 0, 0 });
		nshp->texcoord.push_back(vec2f{ 0, 1 });
		nshp->texcoord.push_back(vec2f{ 1, 1 });
		nshp->texcoord.push_back(vec2f{ 1, 0 });
		nshp->quads.push_back(vec4i{ 0, 1, 2, 3 });
		nshp->mat = mat;
		add_instance(scn, type, frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, nshp, mat);
		
		y += v.at(j)*h;
		printf("splitting\n"); //
	}

	remove_instance(scn, shp);
	return scn;
}

//facade repeat on y-axis
scene* repeat_y(scene* scn, shape* shp, int parts, const std::string& type) {
	if (parts < 1) {
		return scn;
	}
	std::vector<float> v = std::vector<float>();
	for (int i = 0; i < parts; i++) {
		v.push_back(1.0f / (float)parts);
	}
	return split_y(scn, shp, v, type);
}

//facade split on x-axis
scene* split_x(scene* scn, shape* shp, std::vector<float> v, const std::string& type) {
	printf("check\n"); //
	float check = 0;
	for (int i = 0; i < v.size(); i++) {
		check += v.at(i);
	}
	if (check != 1) {
		printf("fail\n"); //
		return scn;
	}

	float y0 = shp->pos.at(0).y; //same y for v0 and v1
	float y1 = shp->pos.at(3).y; //same y for v2 and v3
	float x = shp->pos.at(0).x;  //same x for v0 and v3
	float z = shp->pos.at(0).z;  //same z for v0 and v3
	float w = shp->pos.at(1).x - shp->pos.at(0).x; //width
	float l = shp->pos.at(1).z - shp->pos.at(0).z; //length

	material* mat = new material{ type };
	mat->kd = vec3f{ 1.0f, 1.0f, 1.0f }; //this is only for test
	mat->kd_txt = new texture{ type, "colored.png" };
	//note : a material without texture triggers a segmentation fault because add_instance pushes back a nullptr in a vector

	for (int j = 0; j < v.size(); j++) {
		shape* nshp = new shape();
		nshp->pos.push_back(vec3f{ x, y0, z });
		nshp->pos.push_back(vec3f{ x + v.at(j)*w, y0, z + v.at(j)*l });
		nshp->pos.push_back(vec3f{ x + v.at(j)*w, y1, z + v.at(j)*l });
		nshp->pos.push_back(vec3f{ x, y1, z });
		nshp->texcoord.push_back(vec2f{ 0, 0 });
		nshp->texcoord.push_back(vec2f{ 0, 1 });
		nshp->texcoord.push_back(vec2f{ 1, 1 });
		nshp->texcoord.push_back(vec2f{ 1, 0 });
		nshp->quads.push_back(vec4i{ 0, 1, 2, 3 });
		nshp->mat = mat;
		add_instance(scn, type, frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, nshp, mat);

		x += v.at(j)*w;
		z += v.at(j)*l;

	}

	remove_instance(scn, shp);
	return scn;
}

//facade repeat on x-axis
scene* repeat_x(scene* scn, shape* shp, int parts, const std::string& type) {
	if (parts < 1) {
		return scn;
	}
	std::vector<float> v = std::vector<float>();
	for (int i = 0; i < parts; i++) {
		v.push_back(1.0f / (float)parts);
	}
	return split_x(scn, shp, v, type);
}

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


	//make base
	shape* base = new shape();
	base->pos.push_back(vec3f{ -1,  0, -1 });
	base->pos.push_back(vec3f{ -1,  0,  1 });
	base->pos.push_back(vec3f{  1,  0,  1 });
	base->pos.push_back(vec3f{  1,  0, -1 });

	base->quads.push_back(vec4i{ 0, 1, 2, 3 });

	//make base 2
	shape* base2 = new shape();
	base2->pos.push_back(vec3f{  3,  0, -1 });
	base2->pos.push_back(vec3f{  3,  0,  1 });
	base2->pos.push_back(vec3f{  9,  0,  3 });
	base2->pos.push_back(vec3f{  9,  0, -3 });

	base2->quads.push_back(vec4i{ 0, 1, 2, 3 });

	//make buildings from basement
	material* mat = make_material("building", { 0.0f, 0.0f, 1.0f }, "colored.png");
	shape* building = make_building(base, 4.0f);
	shape* building2 = make_building(base2, 10.0f);

	building->mat = mat;
	building2->mat = mat;

	//translate building
	translate(building, vec3f{ -1,  0, -3 });

	//make a single vertical facade
	shape* facade = new shape();
	facade->pos.push_back(vec3f{ -1,  0,  0 });
	facade->pos.push_back(vec3f{  1,  0,  0 });
	facade->pos.push_back(vec3f{  1,  2,  0 });
	facade->pos.push_back(vec3f{ -1,  2,  0 });

	facade->quads.push_back({ 0, 1, 2, 3 });
	facade->mat = mat;

	//make scene
	scene* scn = init_scene();
	add_instance(scn, "building",  frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, building, mat);
	add_instance(scn, "building", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, building2, mat);
	add_instance(scn, "facade",    frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, facade, mat);

	//split facade test
	repeat_y(scn, facade, 3, "floors");
	//split floors test
	for (instance* inst : scn->instances) {
		if (inst->name == "floors") {
			repeat_x(scn, inst->shp->shapes.at(0), 3, "tile");
		}
	}

	//save
	save_options sopt = save_options();
	printf("saving scene %s\n", sceneout.c_str());
	save_scene(sceneout, scn, sopt );
	delete scn;
	return 0;
}

