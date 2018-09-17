#include <vector>
#include "../yocto/yocto_gl.h"
//#include "../yocto/yocto_gltf.h"

using namespace ygl;

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

//copied from model.cpp
material* make_material(const std::string& name, const vec3f& kd,
	const std::string& kd_txt, const vec3f& ks = { 0.04f, 0.04f, 0.04f },
	float rs = 0.01f) {
	// IL TUO CODICE VA QUA
	material* mat = new material{ name };
	mat->kd = kd;
	mat->kd_txt = new texture{ kd_txt };
	mat->ks = ks;
	mat->rs = rs;
	return mat;
}

//copied from model.cpp
void add_instance(scene* scn, const std::string& name, const frame3f& f,
	shape* shp, material* mat) {
	if (!shp || !mat) return;
	// IL TUO CODICE VA QUA
	instance* inst = new instance{ name, f, mat, shp };
	if (std::find(scn->instances.begin(), scn->instances.end(), inst) == scn->instances.end()) {
		scn->instances.push_back(inst);
	}
	if (std::find(scn->shapes.begin(), scn->shapes.end(), shp) == scn->shapes.end()) {
		scn->shapes.push_back(shp);
	}
	if (std::find(scn->materials.begin(), scn->materials.end(), mat) == scn->materials.end()) {
		scn->materials.push_back(mat);
	}
	if (std::find(scn->textures.begin(), scn->textures.end(), mat->kd_txt) == scn->textures.end()) {
		scn->textures.push_back(mat->kd_txt);
	}
}

//copied from model.cpp
scene* init_scene() {
	auto scn = new scene();
	// add floor
	auto mat = new material{ "floor" };
	mat->kd = { 0.2f, 0.2f, 0.2f };
	mat->kd_txt = new texture{ "grid.png" };
	scn->textures.push_back(mat->kd_txt);
	scn->materials.push_back(mat);
	auto shp = new shape{ "floor" };
	shp->pos = { { -20, 0, -20 },{ 20, 0, -20 },{ 20, 0, 20 },{ -20, 0, 20 } };
	shp->norm = { { 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 } };
	shp->texcoord = { { -10, -10 },{ 10, -10 },{ 10, 10 },{ -10, 10 } };
	shp->triangles = { { 0, 1, 2 },{ 0, 2, 3 } };
	scn->shapes.push_back(shp);
	scn->instances.push_back(new instance{ "floor", identity_frame3f, mat, shp });
	// add light
	auto lshp = new shape{ "light" };
	lshp->pos = { { 1.4f, 8, 6 },{ -1.4f, 8, 6 } };
	lshp->points = { 0, 1 };
	scn->shapes.push_back(lshp);
	auto lmat = new material{ "light" };
	lmat->ke = { 100, 100, 100 };
	scn->materials.push_back(lmat);
	scn->instances.push_back(
		new instance{ "light", identity_frame3f, lmat, lshp });
	// add camera
	auto cam = new camera{ "cam" };
	cam->frame = lookat_frame3f({ 0, 4, 10 }, { 0, 1, 0 }, { 0, 1, 0 });
	cam->fovy = 15 * pif / 180.f;
	cam->aspect = 16.0f / 9.0f;
	cam->aperture = 0;
	cam->focus = length(vec3f{ 0, 4, 10 } -vec3f{ 0, 1, 0 });
	scn->cameras.push_back(cam);
	return scn;
}

int main(int argc, char** argv ) {
	//parsing
	auto parser =
		yu::cmdline::make_parser(argc, argv, "model", "creates simple scenes");
	auto sceneout = yu::cmdline::parse_opts(
		parser, "--output", "-o", "output scene", "out.obj");
	yu::cmdline::check_parser(parser);

	//printf("creating scene %s\n", type.c_str());


	//make base
	shape* base = new shape();
	base->pos.push_back(vec3f{ -1, 0, -1 });
	base->pos.push_back(vec3f{ -1, 0,  1 });
	base->pos.push_back(vec3f{  1, 0,  1 });
	base->pos.push_back(vec3f{  1, 0, -1 });

	base->quads.push_back(vec4i{ 0, 1, 2, 3 });

	//make base 2
	shape* base2 = new shape();
	base2->pos.push_back(vec3f{  3, 0, -1 });
	base2->pos.push_back(vec3f{  3, 0,  1 });
	base2->pos.push_back(vec3f{  9, 0,  3 });
	base2->pos.push_back(vec3f{  9, 0, -3 });

	base2->quads.push_back(vec4i{ 0, 1, 2, 3 });

	//make buildings from basement
	shape* building = make_building(base, 4.0f);
	shape* building2 = make_building(base2, 10.0f);
	material* mat = make_material("building", { 1, 1, 1 }, "colored.png");

	//make scene
	scene* scn = init_scene();
	add_instance(scn, "building", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, building, mat);
	add_instance(scn, "building2", frame3f{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ 0, 1.25f, 0 } }, building2, mat);

	//save
	printf("saving scene %s\n", sceneout.c_str());
	save_scene(sceneout, scn);
	delete scn;
	return 0;
}

