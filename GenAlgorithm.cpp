#include "raylib.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>

// mapa com a png (blau - meta / vermell - checkpoint / verd - out road)
Image mapaImg;

struct Cervell {
    float pesos[3][2];
    Cervell() {
        for (int i = 0; i < 3; i++) for (int j = 0; j < 2; j++)
            pesos[i][j] = GetRandomValue(-100, 100) / 100.0f;
    }
    Cervell Mutar() {
        Cervell fill = *this;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 2; j++) {
                if (GetRandomValue(0, 100) < 30)
                    fill.pesos[i][j] += (GetRandomValue(-50, 50) / 100.0f);
            }
        }
        return fill;
    }

    void Guardar(const char* arxiu) {
        std::ofstream out(arxiu);
        if (out.is_open()) {
            for (int i = 0; i < 3; i++) for (int j = 0; j < 2; j++) out << pesos[i][j] << " ";
            out.close();
        }
    }

    bool Carregar(const char* arxiu) {
        std::ifstream in(arxiu);
        if (in.is_open()) {
            for (int i = 0; i < 3; i++) for (int j = 0; j < 2; j++) in >> pesos[i][j];
            in.close();
            return true;
        }
        return false;
    }
};

struct Cotxe {
    float x, y, angle, velocitat;
    bool estavellat;
    bool ha_acabat;
    bool passat_checkpoint;
    float fitness;
    int frames_inactiu;
    Cervell cervell;
    float sensors[3];
    Color color;

    Cotxe() { Reiniciar(); }

    void Reiniciar() {
        // Ara iniciem a la part superior del circuit ovalat creat
        x = 390.0f;
        y = 115.0f;
        angle = 0.0f; // Mirant cap a la dreta
        velocitat = 0.0f;
        estavellat = false;
        ha_acabat = false;
        passat_checkpoint = false;
        fitness = 0.0f;
        frames_inactiu = 0;
        color = ColorFromHSV(GetRandomValue(0, 360), 0.8f, 0.9f);
    }

    // NOVA FUNCIÓ DE COL·LISIÓ PER PÍXELS
    bool EsDinsPista(float px, float py) {
        int ix = (int)px;
        int iy = (int)py;

        // Si surt de la imatge, s'ha estavellat
        if (ix < 0 || ix >= mapaImg.width || iy < 0 || iy >= mapaImg.height) return false;

        Color pixel = GetImageColor(mapaImg, ix, iy);

        // Si el píxel és predominantment verd (gespa), està fora
        if (pixel.g > 120 && pixel.r < 80 && pixel.b < 80) return false;

        return true; // Qualsevol altre color és pista segura
    }

    float CalcularSensor(float angle_offset) {
        float angle_raig = angle + angle_offset;
        float dist = 0.0f;
        float pas = 2.0f;

        while (dist < 100.0f) {
            float test_x = x + cos(angle_raig) * dist;
            float test_y = y + sin(angle_raig) * dist;
            if (!EsDinsPista(test_x, test_y)) break; // Xoca amb la gespa visual
            dist += pas;
        }
        return dist;
    }

    void Actualitzar(bool es_huma) {
        if (estavellat || ha_acabat) return;

        sensors[0] = CalcularSensor(-0.785f);
        sensors[1] = CalcularSensor(0.0f);
        sensors[2] = CalcularSensor(0.785f);

        float input_accel = 0.0f;
        float input_gir = 0.0f;

        if (es_huma) {
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) input_accel = 1.0f;
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) input_accel = -1.0f;
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) input_gir = -1.0f;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) input_gir = 1.0f;
        }
        else {
            for (int i = 0; i < 3; i++) {
                float sn = sensors[i] / 100.0f;
                input_accel += sn * cervell.pesos[i][0];
                input_gir += sn * cervell.pesos[i][1];
            }
            if (input_accel > 1.0f) input_accel = 1.0f;
            else if (input_accel < 0.0f) input_accel = 0.0f;

            if (input_gir > 1.0f) input_gir = 1.0f;
            else if (input_gir < -1.0f) input_gir = -1.0f;
        }

        velocitat += input_accel * 0.3f;
        velocitat *= 0.92f;

        if (abs(velocitat) > 0.1f) {
            angle += input_gir * 0.1f * (velocitat > 0 ? 1 : -1);
        }

        x += cos(angle) * velocitat;
        y += sin(angle) * velocitat;

        if (velocitat < 0.5f) frames_inactiu++;
        else frames_inactiu = 0;

        if (frames_inactiu > 60 && !es_huma) estavellat = true;

        if (!EsDinsPista(x, y)) {
            estavellat = true;
        }
        else if (!estavellat) {
            fitness += velocitat;

            // sistema de checkpoint
            if (x >= 0 && x < mapaImg.width && y >= 0 && y < mapaImg.height) {
                Color pixel = GetImageColor(mapaImg, (int)x, (int)y);

                // Checkpoint (Color Vermell)
                if (pixel.r > 200 && pixel.g < 80 && pixel.b < 80) {
                    passat_checkpoint = true;
                }
                // Meta (Color Blau) i només si ha passat pel vermell abans
                if (pixel.b > 200 && passat_checkpoint) {
                    ha_acabat = true;
                }
            }
        }
    }

    void Dibuixar(bool mostrar_sensors) {
        if (mostrar_sensors && !estavellat && !ha_acabat) {
            float angles[3] = { -0.785f, 0.0f, 0.785f };
            for (int i = 0; i < 3; i++) {
                float rx = x + cos(angle + angles[i]) * sensors[i];
                float ry = y + sin(angle + angles[i]) * sensors[i];
                DrawLine(x, y, rx, ry, Fade(GREEN, 0.5f));
                DrawCircle(rx, ry, 3, RED);
            }
        }
        Rectangle rect = { x, y, 16, 8 };
        Vector2 origen = { 8, 4 };
        DrawRectanglePro(rect, origen, angle * RAD2DEG, estavellat ? DARKGRAY : (ha_acabat ? GOLD : color));
    }
};

int main() {
    InitWindow(800, 600, "Cotxes evolutius");
    SetTargetFPS(120);

    // Carrega de mapa
    mapaImg = LoadImage("circuit.png");

    // Si la imatge existeix (la teva), la redimensionem per força a 800x600
    if (mapaImg.data != NULL) {
        ImageResize(&mapaImg, 800, 600);
    }
    // Si no troba l'arxiu, el dibuixa per nosaltres i el guarda
    else {
        mapaImg = GenImageColor(800, 600, Color{ 30, 150, 50, 255 }); // Fons Verd fosc (gespa)
        ImageDrawCircle(&mapaImg, 400, 300, 250, GRAY); // Pista gris
        ImageDrawCircle(&mapaImg, 400, 300, 150, Color{ 30, 150, 50, 255 }); // Forat verd al centre

        // Dibuixem el Checkpoint Vermell a sota
        ImageDrawRectangle(&mapaImg, 380, 500, 40, 100, RED);
        // Dibuixem la Línia de Meta Blava a dalt (darrere del cotxe)
        ImageDrawRectangle(&mapaImg, 350, 50, 40, 100, BLUE);

        ExportImage(mapaImg, "circuit.png"); // El guardem
    }

    Texture2D mapaTex = LoadTextureFromImage(mapaImg);

    const int POBLACIO = 100;
    std::vector<Cotxe> cotxes(POBLACIO);
    Cotxe cotxe_huma;
    cotxe_huma.color = WHITE;

    int generacio = 1;
    int mode_joc = 0;
    int frames_generacio = 0;
    int missatge_timer = 0;
    const char* missatge_text = "";
    Color color_missatge = WHITE;

    while (!WindowShouldClose()) {
        if (mode_joc == 0) {
            if (IsKeyPressed(KEY_S)) {
                int millor = 0;
                for (int i = 1; i < POBLACIO; i++) {
                    if (cotxes[i].fitness > cotxes[millor].fitness) millor = i;
                }
                cotxes[millor].cervell.Guardar("millor_cervell.txt");
                missatge_text = "CERVELL GUARDAT CORRECTAMENT!";
                color_missatge = GREEN;
                missatge_timer = 120;
            }

            if (IsKeyPressed(KEY_L)) {
                Cervell carregat;
                if (carregat.Carregar("millor_cervell.txt")) {
                    cotxes[0].cervell = carregat;
                    cotxes[0].color = RED;
                    for (int i = 1; i < POBLACIO; i++) cotxes[i].cervell = carregat.Mutar();
                    for (auto& c : cotxes) c.Reiniciar();
                    generacio = 1;
                    frames_generacio = 0;
                    missatge_text = "CERVELL CARREGAT!";
                    color_missatge = SKYBLUE;
                    missatge_timer = 120;
                }
            }
        }

        if (IsKeyPressed(KEY_P)) {
            mode_joc = (mode_joc == 0) ? 1 : 0;
            cotxe_huma.Reiniciar();
        }

        if (mode_joc == 0) {
            bool tots_morts = true;
            int index_guanyador = -1;
            frames_generacio++;

            for (int i = 0; i < POBLACIO; i++) {
                cotxes[i].Actualitzar(false);
                if (!cotxes[i].estavellat && !cotxes[i].ha_acabat) tots_morts = false;
                if (cotxes[i].ha_acabat && index_guanyador == -1) index_guanyador = i;
            }

            if (index_guanyador != -1 || tots_morts || frames_generacio > 1200) {
                std::vector<Cotxe> nova_generacio(POBLACIO);

                if (index_guanyador != -1) {
                    Cotxe campio = cotxes[index_guanyador];
                    nova_generacio[0].cervell = campio.cervell;
                    nova_generacio[0].color = RED;
                    for (int i = 1; i < POBLACIO; i++) nova_generacio[i].cervell = campio.cervell.Mutar();
                }
                else {
                    std::sort(cotxes.begin(), cotxes.end(), [](const Cotxe& a, const Cotxe& b) {
                        return a.fitness > b.fitness;
                        });
                    nova_generacio[0].cervell = cotxes[0].cervell;
                    nova_generacio[0].color = RED;
                    for (int i = 1; i < POBLACIO; i++) {
                        int pare = GetRandomValue(0, 9);
                        nova_generacio[i].cervell = cotxes[pare].cervell.Mutar();
                    }
                }

                cotxes = nova_generacio;
                generacio++;
                frames_generacio = 0;
            }
        }
        else {
            cotxe_huma.Actualitzar(true);
            if ((cotxe_huma.estavellat || cotxe_huma.ha_acabat) && IsKeyPressed(KEY_R)) cotxe_huma.Reiniciar();
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Dibuixem mapa
        DrawTexture(mapaTex, 0, 0, WHITE);

        if (mode_joc == 0) {
            for (auto& c : cotxes) c.Dibuixar(false);
            if (!cotxes[0].estavellat) cotxes[0].Dibuixar(true);

            DrawText("MODE: Entrenament F1", 10, 10, 20, YELLOW);
            DrawText(TextFormat("Generacio: %d", generacio), 10, 40, 20, WHITE);
            DrawText("Temps Restant:", 10, 70, 15, LIGHTGRAY);
            DrawRectangle(120, 72, 200 - (frames_generacio / 6.0f), 10, RED);

            DrawText("Controls:", 10, 100, 15, BLACK);
            DrawText("'P' -> Conduir", 10, 120, 15, BLACK);
            DrawText("'S' -> Guardar millor IA", 10, 140, 15, BLACK);
            DrawText("'L' -> Carregar IA", 10, 160, 15, BLACK);

            if (missatge_timer > 0) {
                DrawText(missatge_text, 400, 20, 20, color_missatge);
                missatge_timer--;
            }
        }
        else {
            cotxe_huma.Dibuixar(true);
            DrawText("MODE: Huma", 10, 10, 20, ORANGE);

            if (cotxe_huma.ha_acabat) DrawText("HAS GUANYAT! Prem 'R'", 250, 300, 30, GOLD);
            else if (cotxe_huma.estavellat) DrawText("ESTAVELLAT! Prem 'R'", 250, 300, 30, RED);
        }

        EndDrawing();
    }

    UnloadTexture(mapaTex);
    UnloadImage(mapaImg);
    CloseWindow();
    return 0;
}