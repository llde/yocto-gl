#include "scene.h"

#include "ext/yocto_math.h"
#include "ext/yocto_utils.h"


// converte un mesh in vertici sono condivis
// per ogni faccia, aggiunge i rispettivi vertici a mesh e aggiusta
// gli indici di faccia
// alla fine, fa l'update delle normali
// modifica la shape passata e ne ritorna il puntatore
shape* facet_normals(shape* shp) {
    // IL TUO CODICE VA QUA
    std::vector<vec3f> new_positions;
    std::vector<vec3f> new_normals;
    std::vector<vec2f> new_texcoord;

    if(!shp->triangles.empty()){
        std::vector<vec3i> new_triangles;
        for (vec3i& t : shp->triangles) {
            new_triangles.push_back({new_positions.size(), new_positions.size() + 1, new_positions.size() + 2});
            new_positions.push_back(shp->pos[t.x]);
            new_positions.push_back(shp->pos[t.y]);
            new_positions.push_back(shp->pos[t.z]);
            if(!shp->norm.empty()){
                new_normals.push_back(shp->norm[t.x]);
                new_normals.push_back(shp->norm[t.y]);
                new_normals.push_back(shp->norm[t.z]);
            }
            if(!shp->texcoord.empty()){ 
                new_texcoord.push_back(shp->texcoord[t.x]);
                new_texcoord.push_back(shp->texcoord[t.y]);
                new_texcoord.push_back(shp->texcoord[t.z]);
            }
        }
        shp->triangles = new_triangles;
    }
    else if(!shp->quads.empty()){
        std::vector<vec4i> new_quads;
        for (vec4i& t : shp->quads) {
            new_quads.push_back({new_positions.size(), new_positions.size() + 1, new_positions.size() + 3, new_positions.size() + 2});
            new_positions.push_back(shp->pos[t.x]);
            new_positions.push_back(shp->pos[t.y]);
            new_positions.push_back(shp->pos[t.z]);
            new_positions.push_back(shp->pos[t.w]);

            if(!shp->norm.empty()){
                new_normals.push_back(shp->norm[t.x]);
                new_normals.push_back(shp->norm[t.y]);
                new_normals.push_back(shp->norm[t.z]);
                new_normals.push_back(shp->norm[t.w]);
            }
            if(!shp->texcoord.empty()){ 
                new_texcoord.push_back(shp->texcoord[t.x]);
                new_texcoord.push_back(shp->texcoord[t.y]);
                new_texcoord.push_back(shp->texcoord[t.z]);
                new_texcoord.push_back(shp->texcoord[t.w]);
            }
        }
        shp->quads = new_quads;
    }
    shp->pos = new_positions; 
    shp->norm = new_normals; 
    shp->texcoord = new_texcoord;
    return shp;
}

// spota ogni vertice i lungo la normale per una quantità dist_txt[i] * scale
// modifica la shape passata e ne ritorna il puntatore
shape* displace(shape* shp, texture* disp_txt, float scale) {
    // IL TUO CODICE VA QUA
    for(size_t i = 0; i < shp->pos.size(); i++){
        shp->pos.at(i) += shp->norm.at(i) * eval_texture(disp_txt,shp->texcoord.at(i) ,false) * scale;
    }
    compute_smooth_normals(shp);
    return shp;
}

inline vec3f& operator/=(vec3f& a, float b){
    a.x /= b;    
    a.y /= b;    
    a.z /= b;    
    return a;
}
// implementa catmull-clark sudision eseguiendo la suddivisione level volte
// il primo step della suddivisione, che introduce i nuovi vertici e faccie,
// e' implementato con tesselate()
// modifica la shape passata e ne ritorna il puntatore
shape* catmull_clark(shape* shp, int level) {
    // IL TUO CODICE VA QUA
    if(level == 0) return shp;
    tesselate(shp);
    std::vector<vec3f> temp_centr(shp->pos.size(), {0,0,0});
    std::vector<float> count(shp->pos.size(), 0);
    for (vec4i& quad : shp->quads) {
        vec3f centroide = (shp->pos[quad.x] + shp->pos[quad.y] + shp->pos[quad.z] + shp->pos[quad.w]) / 4.0f;
        temp_centr[quad.x] += centroide;
        temp_centr[quad.y] += centroide;
        temp_centr[quad.z] += centroide;
        temp_centr[quad.w] += centroide;
        count[quad.x] += 1;
        count[quad.y] += 1;
        count[quad.z] += 1;
        count[quad.w] += 1;

    }
    for (size_t i = 0; i < shp->pos.size(); i++) {
         temp_centr[i] /= count[i]; 
    }
    for (size_t i = 0; i < shp->pos.size(); i++) {
        shp->pos[i] = shp->pos[i] + (temp_centr[i] - shp->pos[i]) * (4.0f / count[i]);
    }

    return catmull_clark(shp, level -1);
}


shape* generate_quads(const std::string& name, int usteps, int vsteps){
    shape* shp = new shape({name});
    for(size_t i = 0; i < usteps; i++){
        for(size_t j = 0; j < vsteps; j++){
            size_t x_step = j * (usteps + 1) + i;
            size_t y_step = (j+1) * (usteps + 1) + i;
            shp->quads.push_back({x_step,x_step+1, y_step +1 ,y_step});
        }
    }
    return shp;
}

// crea una quadrato nel piano (x,y) con dimensione [-r,r]x[-r,r]
// il piano ha usteps+1 in x and vsteps+1 in y
// le texture coordinates sono sono in [0,1]x[0,1]
// quindi ogni vertice ha pos=[2*u -1, 2*v -1 ,0], norm=[0,0,1], texcooerd=[u,v]
// name e' l'identificativo
// il mesh è fatto di quads con griglia definita da usteps x vsteps
// ogni quad e' definito come [i,j], [i+1,j], [i+1,j+1], [i,j+1]
shape* make_quad(const std::string& name, int usteps, int vsteps, float r) {
    // IL TUO CODICE VA QU
    shape* shp = generate_quads(name,usteps,vsteps);
    for(size_t j = 0; j <= vsteps; j++){
        for(size_t i = 0; i <= usteps; i++){
            float u = (float)i/(float)usteps;
            float v = (float)j/(float)vsteps;
            shp->norm.push_back({0.0f,0.0f,1.0f});
            shp->pos.push_back({r*2*u - 1 , r*2*v -1 , 0});   
            shp->texcoord.push_back({u,v});
        }
    }
    return shp;
}

// crea una sphera di raggio r in coordinate spheriche con (2 pi u, pi v)
// vedi al descrizione di make_quad()
// name e' l'identificativo
shape* make_sphere(const std::string& name, int usteps, int vsteps, float r) {
    shape* shp = generate_quads(name,usteps,vsteps);
    for(size_t j = 0; j <= vsteps; j++){
        for(size_t i = 0; i <= usteps; i++){
            float u = (float)i/(float)usteps;
            float v = (float)j/(float)vsteps;
            vec2f pol = { 2 * pif * u , pif * (1 - v)};
            shp->norm.push_back({cos(pol.x) * sin(pol.y) ,  sin(pol.x) * sin(pol.y) , cos(pol.y)});
            shp->pos.push_back({r * cos(pol.x) * sin(pol.y) ,  r* sin(pol.x) * sin(pol.y) , r*cos(pol.y)});   
            shp->texcoord.push_back({u,v});
        }
    }
    return shp;
}

// crea una geosfera ottenuta tessellando la shape originale con tesselate()
// level volte e poi spontando i vertici risultanti sulla sphere di raggio r
// (i vertici sulla sphera unitaria sono normalize(p))
// name e' l'identificativo
shape* make_geosphere(const std::string& name, int level, float r) {
    const float X = 0.525731112119133606f;
    const float Z = 0.850650808352039932f;
    auto pos = std::vector<vec3f>{{-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z},
        {X, 0.0, -Z}, {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
        {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}};
    auto triangles = std::vector<vec3i>{{0, 1, 4}, {0, 4, 9}, {9, 4, 5},
        {4, 8, 5}, {4, 1, 8}, {8, 1, 10}, {8, 10, 3}, {5, 8, 3}, {5, 3, 2},
        {2, 3, 7}, {7, 3, 10}, {7, 10, 6}, {7, 6, 11}, {11, 6, 0}, {0, 6, 1},
        {6, 10, 1}, {9, 11, 0}, {9, 2, 11}, {9, 5, 2}, {7, 11, 2}};
    // IL TUO CODICE VA QUA
    shape* shp = new shape({name});
    shp->pos = pos;
    shp->triangles = triangles;
    for(size_t i = 0; i < level; i++){
        shp = tesselate(shp);
    }
    for(vec3f& val : shp->pos){
        val = normalize(val)*r;
    }
    return shp;
}

// aggiunge un'instanza alla scena, assicurando che gli oggetti puntati
// dall'instanza (shp, mat, mat->kd_txt) sono anch'essi stati aggiunti alla
// scene almeno una e una sola volta
void add_instance(scene* scn, const std::string& name, const frame3f& quad,
    shape* shp, material* mat) {
    if (!shp || !mat) return;
    // IL TUO COICE VA QUA
    instance* inst = new instance();
    inst->mat = mat;
    inst->shp = shp;
    inst->frame = quad;
    inst->name = name;
    scn->instances.push_back(inst);
    if(std::find(scn->materials.begin(), scn->materials.end(), mat) == scn->materials.end()){
        scn->materials.push_back(mat);
    }
    if(std::find(scn->shapes.begin(), scn->shapes.end(), shp) == scn->shapes.end()){
        scn->shapes.push_back(shp);
    }
    if(std::find(scn->textures.begin(), scn->textures.end(), mat->kd_txt) == scn->textures.end()){
        scn->textures.push_back(mat->kd_txt);
    }
}

// crea an material con i parametrii specifici - name e' l'identificativo
material* make_material(const std::string& name, const vec3f& kd,
    const std::string& kd_txt, const vec3f& ks = {0.04f, 0.04f, 0.04f},
    float rs = 0.01f) {
    // IL TUO CODICE VA QUA
    material* mat = new material();
    mat->name = name;
    mat->kd_txt = new texture{kd_txt};
    mat->kd = kd;
    mat->ks = ks;
    mat->rs = rs;
    return mat;
}

// aggiunge num sphere di raggio r in un cerchio di raggio R oreintato come
// specificato da quad
void add_sphere_instances(
    scene* scn, const frame3f& quad, float R, float r, int num, material* mat) {
    shape* sphere = make_sphere("sphere", 16,8,r);
    float angle_augm = (2.0f * pif)/ num;
    for(size_t i = 0; i <num ; i++){
        frame3f  new_origin = quad;
        new_origin.z = quad.y;
        new_origin.y = quad.x;
        new_origin.x = quad.z;
        new_origin.o.x += std::cos(angle_augm*i)*R;
        new_origin.o.y += std::sin(angle_augm*i)*R;
        add_instance(scn,"spherei", new_origin,sphere,mat);
    }

    // IL TUO CODICE VA QUA
}

scene* init_scene() {
    auto scn = new scene();
    // add floor
    auto mat = new material{"floor"};
    mat->kd = {0.2f, 0.2f, 0.2f};
    mat->kd_txt = new texture{"grid.png"};
    scn->textures.push_back(mat->kd_txt);
    scn->materials.push_back(mat);
    auto shp = new shape{"floor"};
    shp->pos = {{-20, 0, -20}, {20, 0, -20}, {20, 0, 20}, {-20, 0, 20}};
    shp->norm = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}};
    shp->texcoord = {{-10, -10}, {10, -10}, {10, 10}, {-10, 10}};
    shp->triangles = {{0, 1, 2}, {0, 2, 3}};
    scn->shapes.push_back(shp);
    scn->instances.push_back(new instance{"floor", identity_frame3f, mat, shp});
    // add light
    auto lshp = new shape{"light"};
    lshp->pos = {{1.4f, 8, 6}, {-1.4f, 8, 6}};
    lshp->points = {0, 1};
    scn->shapes.push_back(lshp);
    auto lmat = new material{"light"};
    lmat->ke = {100, 100, 100};
    scn->materials.push_back(lmat);
    scn->instances.push_back(
        new instance{"light", identity_frame3f, lmat, lshp});
    // add camera
    auto cam = new camera{"cam"};
    cam->frame = lookat_frame3f({0, 4, 10}, {0, 1, 0}, {0, 1, 0});
    cam->fovy = 15 * pif / 180;
    cam->aspect = 16.0f / 9.0f;
    cam->aperture = 0;
    cam->focus = length(vec3f{0, 4, 10} - vec3f{0, 1, 0});
    scn->cameras.push_back(cam);
    return scn;
}

int main(int argc, char** argv) {
    // command line parsing
    auto parser =
        yu::cmdline::make_parser(argc, argv, "model", "creates simple scenes");
    auto sceneout = yu::cmdline::parse_opts(
        parser, "--output", "-o", "output scene", "out.obj");
    auto type = yu::cmdline::parse_args(
        parser, "type", "type fo scene to create", "empty", true);
    yu::cmdline::check_parser(parser);

    printf("creating scene %s\n", type.c_str());

    // create scene
    auto scn = init_scene();
    if (type == "empty") {
    } else if (type == "simple") {
        add_instance(scn, "quad", make_frame3_fromz({-1.25f, 1, 0}, {0, 0, 1}),
            make_quad("quad", 16, 16, 1),
            make_material("quad", {1, 1, 1}, "colored.png"));
        add_instance(scn, "sphere", make_frame3_fromz({1.25f, 1, 0}, {0, 0, 1}),
            make_sphere("sphere", 32, 16, 1),
            make_material("sphere", {1, 1, 1}, "colored.png"));
    } else if (type == "instances") {
        add_sphere_instances(scn,
            frame3f{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 1.25f, 0}}, 1, 0.1, 16,
            make_material("obj", {1, 1, 1}, "colored.png"));
    } else if (type == "displace") {
        add_instance(scn, "quad1", make_frame3_fromz({-1.25f, 1, 0}, {0, 0, 1}),
            displace(make_quad("quad1", 64, 64, 1), make_grid_texture(256, 256),
                0.5),
            make_material("quad1", {1, 1, 1}, "colored.png"));
        add_instance(scn, "quad2", make_frame3_fromz({1.25f, 1, 0}, {0, 0, 1}),
            displace(make_quad("quad2", 64, 64, 1),
                make_bumpdimple_texture(256, 256), 0.5),
            make_material("quad2", {1, 1, 1}, "colored.png"));
    } else if (type == "normals") {
        add_instance(scn, "smnooth",
            make_frame3_fromz({-1.25f, 1, 0}, {0, 0, 1}),
            make_geosphere("smnooth", 2, 1),
            make_material("smnooth", {0.5f, 0.2f, 0.2f}, ""));
        add_instance(scn, "faceted",
            make_frame3_fromz({1.25f, 1, 0}, {0, 0, 1}),
            facet_normals(make_geosphere("faceted", 2, 1)),
            make_material("faceted", {0.2f, 0.5f, 0.2f}, ""));
    } else if (type == "subdiv") {
        add_instance(scn, "cube",
            make_frame3_fromzx({-1.25f, 1, 0}, {0, 0, 1}, {1, 0, 0}),
            catmull_clark(make_cube("cube"), 4),
            make_material("cube", {0.5f, 0.2f, 0.2f}, ""));
        add_instance(scn, "monkey",
            make_frame3_fromzx({1.25f, 1, 0}, {0, 0, 1}, {1, 0, 0}),
            catmull_clark(make_monkey("monkey"), 2),
            make_material("monkey", {0.2f, 0.5f, 0.2f}, ""));
    } else {
        throw std::runtime_error("bad scene type");
    }

    // save
    printf("saving scene %s\n", sceneout.c_str());
    save_scene(sceneout, scn);
    delete scn;
    return 0;
}
