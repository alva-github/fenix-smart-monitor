
#include <Arduino.h>
#include <Wire.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "Arduino_GFX_Library.h"
#include "pin_config.h"

// ============================================
// FENIX SMART MONITOR v0.2
// Waveshare ESP32-S3 - Pantalla 480x480
// Arduino_GFX + LVGL 8.4
// ============================================

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 480
#define BUFFER_LINES 40

// ============================================
// CONFIGURACION DE HARDWARE
// ============================================

Arduino_XCA9554SWSPI *expander =
    new Arduino_XCA9554SWSPI(
        7, 0, 2, 1, &Wire, 0x20);

Arduino_ESP32RGBPanel *rgbpanel =
    new Arduino_ESP32RGBPanel(
        17, 3, 46, 9,
        10, 11, 12, 13, 14,
        21, 8, 18, 45, 38, 39,
        40, 41, 42, 2, 1,
        1, 10, 8, 50,
        1, 10, 8, 20);

Arduino_RGB_Display *gfx =
    new Arduino_RGB_Display(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        rgbpanel,
        0,
        true,
        expander,
        GFX_NOT_DEFINED,
        st7701_type1_init_operations,
        sizeof(st7701_type1_init_operations));

// ============================================
// VARIABLES LVGL
// ============================================

static lv_color_t *lv_buffer = nullptr;
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

static lv_obj_t *label_temperatura = nullptr;
static lv_obj_t *label_humedad = nullptr;

static float temperatura = 24.5f;
static int humedad = 58;

static lv_color_t color_fondo;
static lv_color_t color_tarjeta;
static lv_color_t color_celeste;
static lv_color_t color_verde;
static lv_color_t color_secundario;

// ============================================
// TRANSFERENCIA DE IMAGEN A LA PANTALLA
// ============================================

void pantalla_flush(
    lv_disp_drv_t *disp,
    const lv_area_t *area,
    lv_color_t *color_p)
{
    int ancho = area->x2 - area->x1 + 1;
    int alto = area->y2 - area->y1 + 1;

    gfx->draw16bitRGBBitmap(
        area->x1,
        area->y1,
        reinterpret_cast<uint16_t *>(color_p),
        ancho,
        alto);

    lv_disp_flush_ready(disp);
}

// ============================================
// CREACION DE TARJETAS
// ============================================

lv_obj_t *crear_tarjeta(
    lv_obj_t *padre,
    int x,
    int y,
    int ancho,
    int alto)
{
    lv_obj_t *tarjeta = lv_obj_create(padre);

    lv_obj_set_pos(tarjeta, x, y);
    lv_obj_set_size(tarjeta, ancho, alto);

    lv_obj_set_style_bg_color(
        tarjeta, color_tarjeta, 0);

    lv_obj_set_style_bg_opa(
        tarjeta, LV_OPA_COVER, 0);

    lv_obj_set_style_radius(
        tarjeta, 18, 0);

    lv_obj_set_style_border_width(
        tarjeta, 0, 0);

    lv_obj_set_style_pad_all(
        tarjeta, 0, 0);

    lv_obj_clear_flag(
        tarjeta, LV_OBJ_FLAG_SCROLLABLE);

    return tarjeta;
}

// ============================================
// CREACION DE TEXTOS
// ============================================

lv_obj_t *crear_texto(
    lv_obj_t *padre,
    const char *texto,
    int x,
    int y,
    lv_color_t color,
    const lv_font_t *fuente)
{
    lv_obj_t *label = lv_label_create(padre);

    lv_label_set_text(label, texto);
    lv_obj_set_pos(label, x, y);

    lv_obj_set_style_text_color(
        label, color, 0);

    lv_obj_set_style_text_font(
        label, fuente, 0);

    return label;
}

// ============================================
// DASHBOARD PRINCIPAL
// ============================================

void crear_dashboard()
{
    lv_obj_t *pantalla = lv_scr_act();

    lv_obj_set_style_bg_color(
        pantalla, color_fondo, 0);

    lv_obj_set_style_bg_opa(
        pantalla, LV_OPA_COVER, 0);

    lv_obj_clear_flag(
        pantalla, LV_OBJ_FLAG_SCROLLABLE);

    // ----------------------------------------
    // CABECERA
    // ----------------------------------------

    crear_texto(
        pantalla,
        "FENIX",
        35, 25,
        color_celeste,
        &lv_font_montserrat_32);

    crear_texto(
        pantalla,
        "SMART MONITOR",
        35, 75,
        color_secundario,
        &lv_font_montserrat_16);

    // Linea horizontal
    lv_obj_t *linea = lv_obj_create(pantalla);

    lv_obj_set_pos(linea, 35, 115);
    lv_obj_set_size(linea, 410, 2);

    lv_obj_set_style_bg_color(
        linea, color_celeste, 0);

    lv_obj_set_style_bg_opa(
        linea, LV_OPA_COVER, 0);

    lv_obj_set_style_border_width(
        linea, 0, 0);

    lv_obj_set_style_radius(
        linea, 0, 0);

    lv_obj_set_style_pad_all(
        linea, 0, 0);

    // ----------------------------------------
    // TEMPERATURA
    // ----------------------------------------

    lv_obj_t *tarjeta_temp =
        crear_tarjeta(
            pantalla, 25, 145, 205, 160);

    crear_texto(
        tarjeta_temp,
        "TEMPERATURA",
        15, 20,
        color_secundario,
        &lv_font_montserrat_16);

    label_temperatura = crear_texto(
        tarjeta_temp,
        "24.5 C",
        15, 75,
        color_celeste,
        &lv_font_montserrat_32);

    // ----------------------------------------
    // HUMEDAD
    // ----------------------------------------

    lv_obj_t *tarjeta_hum =
        crear_tarjeta(
            pantalla, 250, 145, 205, 160);

    crear_texto(
        tarjeta_hum,
        "HUMEDAD",
        20, 20,
        color_secundario,
        &lv_font_montserrat_16);

    label_humedad = crear_texto(
        tarjeta_hum,
        "58%",
        20, 75,
        color_verde,
        &lv_font_montserrat_32);

    // ----------------------------------------
    // ESTADO DEL SISTEMA
    // ----------------------------------------

    lv_obj_t *tarjeta_estado =
        crear_tarjeta(
            pantalla, 25, 330, 430, 100);

    lv_obj_t *indicador =
        lv_obj_create(tarjeta_estado);

    lv_obj_set_size(indicador, 18, 18);
    lv_obj_set_pos(indicador, 25, 40);

    lv_obj_set_style_radius(
        indicador, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_style_bg_color(
        indicador, color_verde, 0);

    lv_obj_set_style_bg_opa(
        indicador, LV_OPA_COVER, 0);

    lv_obj_set_style_border_width(
        indicador, 0, 0);

    lv_obj_set_style_pad_all(
        indicador, 0, 0);

    crear_texto(
        tarjeta_estado,
        "SISTEMA ACTIVO",
        60, 20,
        lv_color_white(),
        &lv_font_montserrat_20);

    crear_texto(
        tarjeta_estado,
        "FENIX v0.2 - LVGL",
        60, 58,
        color_celeste,
        &lv_font_montserrat_14);
}

// ============================================
// ACTUALIZACION AUTOMATICA
// ============================================

void actualizar_datos(lv_timer_t *timer)
{
    (void)timer;

    temperatura += 0.1f;
    humedad++;

    if (temperatura > 29.0f)
    {
        temperatura = 22.0f;
    }

    if (humedad > 75)
    {
        humedad = 45;
    }

    lv_label_set_text_fmt(
        label_temperatura,
        "%.1f C",
        temperatura);

    lv_label_set_text_fmt(
        label_humedad,
        "%d%%",
        humedad);
}

// ============================================
// SETUP
// ============================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Wire.begin(47, 48);

#ifdef GFX_EXTRA_PRE_INIT
    GFX_EXTRA_PRE_INIT();
#endif

    expander->pinMode(5, OUTPUT);
    expander->pinMode(6, OUTPUT);

    expander->digitalWrite(6, LOW);
    delay(200);

    expander->digitalWrite(5, LOW);
    delay(200);

    expander->digitalWrite(5, HIGH);
    delay(200);

    if (!gfx->begin())
    {
        Serial.println(
            "ERROR: pantalla no inicializada");
        return;
    }

    gfx->fillScreen(BLACK);

    // Inicializacion LVGL
    lv_init();

    // Buffer grafico en PSRAM
    lv_buffer = static_cast<lv_color_t *>(
        heap_caps_malloc(
            SCREEN_WIDTH *
                BUFFER_LINES *
                sizeof(lv_color_t),
            MALLOC_CAP_SPIRAM |
                MALLOC_CAP_8BIT));

    if (lv_buffer == nullptr)
    {
        Serial.println(
            "ERROR: no se pudo reservar PSRAM");
        return;
    }

    lv_disp_draw_buf_init(
        &draw_buf,
        lv_buffer,
        nullptr,
        SCREEN_WIDTH * BUFFER_LINES);

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = pantalla_flush;
    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);

    // Colores del dashboard
    color_fondo = lv_color_hex(0x0C1220);
    color_tarjeta = lv_color_hex(0x1B263A);
    color_celeste = lv_color_hex(0x00D2EB);
    color_verde = lv_color_hex(0x46E196);
    color_secundario = lv_color_hex(0xCBD5E1);

    // Crear interfaz
    crear_dashboard();

    // Actualizar datos cada 2 segundos
    lv_timer_create(
        actualizar_datos,
        2000,
        nullptr);

    Serial.println(
        "FENIX Smart Monitor v0.2 iniciado");
}

// ============================================
// LOOP
// ============================================

void loop()
{
    static uint32_t ultimo = millis();

    uint32_t ahora = millis();
    uint32_t transcurrido = ahora - ultimo;

    ultimo = ahora;

    lv_tick_inc(transcurrido);
    lv_timer_handler();

    delay(5);
}
