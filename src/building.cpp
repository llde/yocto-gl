#include <vector>
#include "../yocto/yocto_gl.h"
//#include "../yocto/yocto_gltf.h"

using namespace ygl;

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
	if (std::find(scn->textures.begin(), scn->textures.end(), mat->kd_txt) == scn->textures.end()) {
		scn->textures.push_back(mat->kd_txt);
	}
}

void remove_instance(scene* scn, shape* shp) {
	//remove shape and instance
	//material and texture?

	std::vector<shape_group*>::iterator shp_i;
	std::vector<instance*>::iterator inst_i;

	//assuming every shape_group containst one shape / every shape is stored in a different group
	for (shp_i = scn->shapes.begin(); shp_i < scn->shapes.end(); shp_i++) {
		if ((*shp_i)->shapes.at(0) == shp) {
			shp_i = scn->shapes.erase(shp_i);
			printf("removing\n");
		}
	}

	for (inst_i = scn->instances.begin(); inst_i < scn->instances.end(); inst_i++) {
		if ((*inst_i)->shp->shapes.at(0) == shp) {
			inst_i = scn->instances.erase(inst_i);
		}
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

//vertical facade splitting
scene* split(scene* scn, shape* shp, std::vector<float> v) {
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

	for (int j = 0; j < v.size(); j++) {
		material* mat = new material{ "facade2" };
		mat->kd = vec3f{ 0.6f, 0.2f, 0.2f }; //this is only for test
		shape* nshp = new shape();
		nshp->pos.push_back(vec3f{ x0, y, z0 });
		nshp->pos.push_back(vec3f{ x1, y, z1 });
		nshp->pos.push_back(vec3f{ x1, y + v.at(j)*h, z1 });
		nshp->pos.push_back(vec3f{ x0, y + v.at(j)*h, z0 });
		nshp->quads.push_back(vec4i{ 0, 1, 2, 3 });
		nshp->mat = shp->mat;
		add_instance(scn, "facade2", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, nshp, nshp->mat);
		//must add texture
		y = y + v.at(j)*h;
		printf("splitting\n"); //
	}

	remove_instance(scn, shp);
	return scn;
}

//copied from model.cpp
material* make_material(const std::string& name, const vec3f& kd,
	const std::string& kd_txt, const vec3f& ks = { 0.04f, 0.04f, 0.04f },
	float rs = 0.01f) {

	material* mat = new material{ name };
	mat->kd = kd;
	mat->kd_txt = new texture{ kd_txt };
	mat->ks = ks;
	mat->rs = rs;
	return mat;
}


//modified from model.cpp
scene* init_scene() {
	auto scn = new scene();
	// add floor
	auto mat = new material{ "floor" };
	mat->kd = { 0.2f, 0.2f, 0.2f };
	mat->kd_txt = new texture{ "grid", "textures/grid.png" };
	scn->textures.push_back(mat->kd_txt);
	scn->materials.push_back(mat);
	auto shp = new shape{ "floor" };
	shp->mat = mat;
	shp->pos = { { -20, 0, -20 },{ 20, 0, -20 },{ 20, 0, 20 },{ -20, 0, 20 } };
	shp->norm = { { 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 } };
	shp->texcoord = { { -10, -10 },{ 10, -10 },{ 10, 10 },{ -10, 10 } };
	shp->triangles = { { 0, 1, 2 },{ 0, 2, 3 } };
	shape_group* shpgrp = new shape_group{};
	shpgrp->shapes.push_back(shp);
	scn->shapes.push_back(shpgrp);
	scn->instances.push_back(new instance{ "floor", identity_frame3f, shpgrp });
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
	material* mat = make_material("building", { 0.23f, 0.11f, 0.76f }, "textures/colored.png");
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
	scn->materials.push_back(mat);
	scn->textures.push_back(mat->kd_txt);
	add_instance(scn, "building",  frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, building, mat);
	add_instance(scn, "building2", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, building2, mat);
	add_instance(scn, "facade",    frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, facade, mat);

	//split facade
	split(scn, facade, std::vector<float>{ 0.2, 0.1, 0.7 });

	//save
	save_options sopt = save_options();
	printf("saving scene %s\n", sceneout.c_str());
	save_scene(sceneout, scn, sopt );
	delete scn;
	return 0;
}

