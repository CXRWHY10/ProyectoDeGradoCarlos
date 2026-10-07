#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MAX31865.h>

// =======================================================
// CONFIGURACIÓN DE TU RED WI-FI Y CREDENCIALES WEB
// =======================================================
const char* ssid     = "ProyectoCarlos";
const char* password = "carlitos10";

// Credenciales para iniciar sesión en la interfaz web 
const char* www_username = "admin";
const char* www_password = "1234";

// Servidor Web escuchando en el puerto estándar 80
WebServer server(80);

// =======================================================
// CONFIGURACIÓN DE HARDWARE (Pines)
// =======================================================
// Pantalla LCD 16x2 en la dirección I2C 0x27 (pines SDA=21, SCL=22 por defecto en ESP32)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Módulo PT100 (MAX31865) usando comunicación SPI por Hardware
// Asegúrate de conectar CS al 5, SDI al 23, SDO al 19 y CLK al 18
#define MAX31865_CS   5
#define MAX31865_SDI  23
#define MAX31865_SDO  19
#define MAX31865_CLK  18

// Referencias del sensor PT100
#define RREF          430.0
#define RNOMINAL      100.0

Adafruit_MAX31865 thermo = Adafruit_MAX31865(MAX31865_CS, MAX31865_SDI, MAX31865_SDO, MAX31865_CLK);

// Entrada digital aislada (viene del Optoacoplador conectado al PLC de 24V)
const int PIN_PLC = 34;

// =======================================================
// VARIABLES DEL SISTEMA (Estado y Tiempos)
// =======================================================
unsigned long tiempoInicio = 0;
unsigned long tiempoTranscurrido = 0;
bool sistemaActivo = false;
bool estadoAnteriorPLC = LOW;
float temperaturaActual = 0.0;
unsigned long ultimoTiempoLectura = 0; // Para no leer el sensor constantemente

// =======================================================
// INTERFAZ WEB (HTML + CSS + JS)
// =======================================================
// Incluye pantalla de login y simulación gráfica interactiva del tanque con motor rotativo y líquido dinámico.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Planta de Mezclado - Control Industrial</title>
  <style>
    :root {
      --bg: #0f172a;
      --card-bg: #1e293b;
      --text: #f8fafc;
      --accent: #38bdf8;
      --success: #4ade80;
      --danger: #f87171;
      --warning: #fbbf24;
      --border: #334155;
    }
    * { box-sizing: border-box; font-family: 'Segoe UI', system-ui, -apple-system, sans-serif; }
    body {
      background-color: var(--bg);
      color: var(--text);
      margin: 0;
      padding: 1.5rem;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
    }

    /* PANTALLA DE LOGIN */
    .login-card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 1rem;
      padding: 2rem;
      width: 100%;
      max-width: 380px;
      box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.5);
    }
    .login-header { text-align: center; margin-bottom: 1.5rem; }
    .login-header h2 { color: var(--accent); margin: 0 0 0.5rem 0; font-size: 1.6rem; }
    .login-header p { color: #94a3b8; font-size: 0.9rem; margin: 0; }
    .form-group { margin-bottom: 1.2rem; text-align: left; }
    .form-group label { display: block; font-size: 0.85rem; color: #cbd5e1; margin-bottom: 0.4rem; }
    .form-group input {
      width: 100%; padding: 0.75rem 1rem; border-radius: 0.5rem; border: 1px solid var(--border);
      background: #0f172a; color: #fff; font-size: 1rem; outline: none; transition: border 0.2s;
    }
    .form-group input:focus { border-color: var(--accent); }
    .btn-login {
      width: 100%; padding: 0.8rem; border-radius: 0.5rem; border: none; background: #0284c7;
      color: white; font-size: 1rem; font-weight: bold; cursor: pointer; transition: background 0.2s;
    }
    .btn-login:hover { background: #0369a1; }
    .error-msg { color: var(--danger); font-size: 0.85rem; margin-top: 0.8rem; text-align: center; display: none; }

    /* PANEL PRINCIPAL (DASHBOARD) */
    .dashboard { display: none; width: 100%; max-width: 900px; }
    .navbar {
      display: flex; justify-content: space-between; align-items: center; width: 100%;
      background: var(--card-bg); padding: 1rem 1.5rem; border-radius: 0.75rem;
      border: 1px solid var(--border); margin-bottom: 1.5rem;
    }
    .navbar h1 { color: var(--accent); font-size: 1.3rem; margin: 0; }
    .btn-logout {
      background: #334155; color: #f1f5f9; border: none; padding: 0.5rem 1rem;
      border-radius: 0.4rem; cursor: pointer; font-size: 0.85rem; transition: background 0.2s;
    }
    .btn-logout:hover { background: #475569; }

    .main-grid { display: grid; grid-template-columns: 1fr; gap: 1.5rem; width: 100%; }
    @media (min-width: 768px) { .main-grid { grid-template-columns: 1fr 1fr; } }

    /* SIMULACIÓN GRÁFICA DEL TANQUE */
    .tank-card {
      background: var(--card-bg); border: 1px solid var(--border); border-radius: 1rem;
      padding: 1.5rem; display: flex; flex-direction: column; align-items: center; justify-content: center;
    }
    .tank-title { font-size: 1.1rem; color: #94a3b8; margin-bottom: 1rem; font-weight: 600; }
    .tank-container {
      position: relative; width: 220px; height: 260px; display: flex; justify-content: center;
    }
    
    /* Motor Superior */
    .motor {
      position: absolute; top: 0; width: 64px; height: 35px;
      background: linear-gradient(90deg, #334155, #64748b, #334155);
      border-radius: 6px 6px 0 0; border: 2px solid #475569; z-index: 5;
      display: flex; justify-content: center; align-items: center;
    }
    .motor-led {
      width: 10px; height: 10px; border-radius: 50%; background-color: var(--danger);
      box-shadow: 0 0 8px var(--danger); transition: all 0.3s;
    }
    .motor-led.active { background-color: var(--success); box-shadow: 0 0 10px var(--success); }

    /* Eje y Aspas del Agitador */
    .shaft {
      position: absolute; top: 35px; width: 6px; height: 170px; background: #cbd5e1;
      z-index: 4; left: calc(50% - 3px);
    }
    .blades {
      position: absolute; bottom: 60px; left: calc(50% - 40px); width: 80px; height: 18px;
      z-index: 4; background: #94a3b8; border-radius: 4px; transform-origin: center center;
    }
    .blades.spinning { animation: spin 0.7s linear infinite; }
    @keyframes spin { 100% { transform: rotateY(360deg); } }

    /* Tanque Vasija */
    .vessel {
      position: absolute; top: 30px; width: 180px; height: 220px;
      border: 4px solid #64748b; border-bottom-left-radius: 40px; border-bottom-right-radius: 40px;
      border-top-left-radius: 8px; border-top-right-radius: 8px;
      overflow: hidden; background: rgba(15, 23, 42, 0.8); box-shadow: inset 0 0 20px rgba(0,0,0,0.8);
    }

    /* Líquido Aceite/Jabón */
    .liquid {
      position: absolute; bottom: 0; width: 100%; height: 75%;
      background: rgba(56, 189, 248, 0.7); transition: background 0.8s ease;
      display: flex; align-items: center; justify-content: center;
    }
    .liquid-wave {
      position: absolute; top: -10px; width: 200%; height: 20px;
      background: rgba(255, 255, 255, 0.25); border-radius: 40%; display: none;
    }
    .liquid-wave.active { display: block; animation: wave 2.5s infinite linear; }
    @keyframes wave { 0% { transform: translateX(0) rotate(0deg); } 100% { transform: translateX(-50%) rotate(360deg); } }

    .liquid-badge {
      position: relative; z-index: 6; background: rgba(15, 23, 42, 0.85);
      padding: 0.4rem 0.8rem; border-radius: 1rem; border: 1px solid rgba(255,255,255,0.2);
      font-weight: bold; font-size: 0.95rem; color: #fff; text-shadow: 0 1px 2px #000;
    }

    /* TARJETAS DE DATOS */
    .cards-container { display: flex; flex-direction: column; gap: 1.2rem; }
    .card {
      background: var(--card-bg); padding: 1.2rem 1.5rem; border-radius: 0.8rem;
      border: 1px solid var(--border); text-align: left;
    }
    .card h3 { margin: 0 0 0.5rem 0; font-size: 0.95rem; color: #94a3b8; font-weight: normal; }
    .val-large { font-size: 2.2rem; font-weight: bold; color: var(--warning); }
    .status-badge {
      display: inline-block; padding: 0.4rem 1rem; border-radius: 2rem; font-weight: bold; font-size: 1.1rem;
    }
    .status-on { background: rgba(74, 222, 128, 0.15); color: var(--success); border: 1px solid var(--success); }
    .status-off { background: rgba(248, 113, 113, 0.15); color: var(--danger); border: 1px solid var(--danger); }
  </style>
</head>
<body>

  <!-- PANTALLA DE INICIO DE SESIÓN -->
  <div id="login-container" class="login-card">
    <div class="login-header">
      <h2>Jabonería Industrial</h2>
      <p>Acceso al Panel del Mezclador</p>
    </div>
    <form id="login-form" onsubmit="event.preventDefault(); doLogin();">
      <div class="form-group">
        <label for="username">Usuario</label>
        <input type="text" id="username" placeholder="Ingresa usuario" required>
      </div>
      <div class="form-group">
        <label for="password">Contraseña</label>
        <input type="password" id="password" placeholder="••••••••" required>
      </div>
      <button type="submit" class="btn-login">Iniciar Sesión</button>
      <div id="login-error" class="error-msg">Usuario o contraseña incorrectos</div>
    </form>
  </div>

  <!-- PANEL DE CONTROL (SOLO VISIBLE TRAS LOGIN) -->
  <div id="dashboard-container" class="dashboard">
    <div class="navbar">
      <h1>● Panel del Mezclador</h1>
      <button class="btn-logout" onclick="doLogout()">Cerrar Sesión</button>
    </div>

    <div class="main-grid">
      <!-- Tanque Gráfico -->
      <div class="tank-card">
        <div class="tank-title">Simulación del Tanque</div>
        <div class="tank-container">
          <div class="motor">
            <div id="motor-led" class="motor-led"></div>
          </div>
          <div class="vessel">
            <div id="shaft" class="shaft"></div>
            <div id="blades" class="blades"></div>
            <div id="liquid" class="liquid">
              <div id="wave" class="liquid-wave"></div>
              <span id="tank-temp-badge" class="liquid-badge">--.- °C</span>
            </div>
          </div>
        </div>
      </div>

      <!-- Métricas -->
      <div class="cards-container">
        <div class="card">
          <h3>Estado del Motor</h3>
          <div id="status-badge" class="status-badge status-off">○ DETENIDO</div>
        </div>
        <div class="card">
          <h3>Temperatura del Aceite (PT100)</h3>
          <div id="temp-val" class="val-large">--.- °C</div>
        </div>
        <div class="card">
          <h3>Tiempo de Proceso</h3>
          <div id="time-val" class="val-large" style="color: var(--accent);">00:00</div>
        </div>
      </div>
    </div>
  </div>

  <script>
    let pollInterval = null;

    // Verificar si ya hay una sesión activa guardada
    window.addEventListener('DOMContentLoaded', () => {
      if (sessionStorage.getItem('esp32_auth') === 'true') {
        showDashboard();
      }
    });

    function doLogin() {
      const u = document.getElementById('username').value;
      const p = document.getElementById('password').value;
      const errEl = document.getElementById('login-error');

      fetch('/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'username=' + encodeURIComponent(u) + '&password=' + encodeURIComponent(p)
      })
      .then(res => {
        if (res.ok) {
          sessionStorage.setItem('esp32_auth', 'true');
          errEl.style.display = 'none';
          showDashboard();
        } else {
          errEl.innerText = 'Usuario o contraseña incorrectos';
          errEl.style.display = 'block';
        }
      })
      .catch(() => {
        errEl.innerText = 'Error de comunicación con la ESP32';
        errEl.style.display = 'block';
      });
    }

    function doLogout() {
      sessionStorage.removeItem('esp32_auth');
      if (pollInterval) clearInterval(pollInterval);
      document.getElementById('dashboard-container').style.display = 'none';
      document.getElementById('login-container').style.display = 'block';
      document.getElementById('password').value = '';
    }

    function showDashboard() {
      document.getElementById('login-container').style.display = 'none';
      document.getElementById('dashboard-container').style.display = 'block';
      updateData();
      if (!pollInterval) {
        pollInterval = setInterval(updateData, 1000);
      }
    }

    function updateData() {
      fetch('/datos')
        .then(r => r.json())
        .then(data => {
          // Actualizar lectura de Temperatura
          const temp = data.temperatura.toFixed(1);
          document.getElementById('temp-val').innerText = temp + ' °C';
          document.getElementById('tank-temp-badge').innerText = temp + ' °C';

          // Color del líquido según temperatura
          const liquidEl = document.getElementById('liquid');
          if (data.temperatura < 35) {
            liquidEl.style.background = 'rgba(56, 189, 248, 0.75)'; // Azul frío
          } else if (data.temperatura < 55) {
            liquidEl.style.background = 'rgba(245, 158, 11, 0.75)'; // Ámbar / Aceite tibio
          } else {
            liquidEl.style.background = 'rgba(239, 68, 68, 0.75)';  // Rojo caliente
          }

          // Animación del tanque y leds según estado del PLC
          const statusEl = document.getElementById('status-badge');
          const motorLed = document.getElementById('motor-led');
          const blades = document.getElementById('blades');
          const wave = document.getElementById('wave');

          if (data.activo) {
            statusEl.innerText = '● MEZCLANDO';
            statusEl.className = 'status-badge status-on';
            motorLed.className = 'motor-led active';
            blades.className = 'blades spinning';
            wave.className = 'liquid-wave active';
          } else {
            statusEl.innerText = '○ DETENIDO';
            statusEl.className = 'status-badge status-off';
            motorLed.className = 'motor-led';
            blades.className = 'blades';
            wave.className = 'liquid-wave';
          }

          // Formatear Tiempo Transcurrido
          let totalSecs = Math.floor(data.tiempo / 1000);
          let m = Math.floor(totalSecs / 60);
          let s = totalSecs % 60;
          let mStr = m < 10 ? '0' + m : m;
          let sStr = s < 10 ? '0' + s : s;
          document.getElementById('time-val').innerText = mStr + ':' + sStr;
        })
        .catch(err => console.error(err));
    }
  </script>
</body>
</html>
)rawliteral";

// =======================================================
// FUNCIONES DEL SERVIDOR WEB
// =======================================================

// Servir la página principal
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// Validar credenciales enviadas desde el formulario de Login
void handleLogin() {
  if (server.hasArg("username") && server.hasArg("password")) {
    if (server.arg("username") == www_username && server.arg("password") == www_password) {
      server.send(200, "application/json", "{\"status\":\"ok\"}");
      return;
    }
  }
  server.send(401, "application/json", "{\"status\":\"unauthorized\"}");
}

// Enviar estado del PLC, temperatura y tiempo en JSON
void handleDatos() {
  String json = "{";
  json += "\"temperatura\":" + String(temperaturaActual, 1) + ",";
  json += "\"activo\":" + String(sistemaActivo ? "true" : "false") + ",";
  json += "\"tiempo\":" + String(sistemaActivo ? tiempoTranscurrido : 0);
  json += "}";
  server.send(200, "application/json", json);
}

// =======================================================
// INICIALIZACIÓN (Setup)
// =======================================================
void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_PLC, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  thermo.begin(MAX31865_3WIRE); 

  WiFi.begin(ssid, password);
  lcd.setCursor(0, 1);
  lcd.print("Conectando WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi OK IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  
  delay(4000); 
  lcd.clear();

  // Configurar rutas del servidor web
  server.on("/", handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/datos", handleDatos);
  
  server.begin();
  Serial.println("Servidor web iniciado.");
}

// =======================================================
// BUCLE PRINCIPAL (Loop)
// =======================================================
void loop() {
  // Procesar clientes web entrantes
  server.handleClient();

  // 7.1 Lógica del PLC y Cronómetro
  bool estadoActualPLC = digitalRead(PIN_PLC);

  // Si detectamos que el PLC se encendió (Flanco de subida)
  if (estadoActualPLC == HIGH && estadoAnteriorPLC == LOW) {
    sistemaActivo = true;
    tiempoInicio = millis(); // Guardar la marca de tiempo de inicio
  }

  // Si detectamos que el PLC se apagó (Flanco de bajada)
  if (estadoActualPLC == LOW && estadoAnteriorPLC == HIGH) {
    sistemaActivo = false;
  }

  // Si está mezclando, calcular la diferencia de tiempo
  if (sistemaActivo) {
    tiempoTranscurrido = millis() - tiempoInicio;
  }

  estadoAnteriorPLC = estadoActualPLC; // Actualizar memoria del estado

  // 7.2 Lectura de Temperatura (cada 500ms para no saturar)
  if (millis() - ultimoTiempoLectura > 500) {
    // thermo.temperature lee la resistencia y la convierte a grados centígrados
    temperaturaActual = thermo.temperature(RNOMINAL, RREF);
    ultimoTiempoLectura = millis();
    
    // Solo actualizamos la LCD cuando leemos la temperatura para evitar parpadeos
    actualizarPantallaLCD(); 
  }
}

// =======================================================
// ACTUALIZACIÓN DE PANTALLA LCD FÍSICA
// =======================================================
void actualizarPantallaLCD() {
  // --- Línea 1: Temperatura y Estado ---
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperaturaActual, 1); // 1 decimal
  lcd.print((char)223);            // Carácter de grado (°)
  lcd.print("C  ");                // Espacios para borrar números residuales

  lcd.setCursor(10, 0);
  if (sistemaActivo) {
    lcd.print("[ RUN]");
  } else {
    lcd.print("[STOP]");
  }

  // --- Línea 2: Cronómetro ---
  unsigned long seg_totales = 0;
  if (sistemaActivo) {
    seg_totales = tiempoTranscurrido / 1000;
  }
  unsigned int min = seg_totales / 60;
  unsigned int seg = seg_totales % 60;

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  if (min < 10) lcd.print("0");
  lcd.print(min);
  lcd.print("m ");
  if (seg < 10) lcd.print("0");
  lcd.print(seg);
  lcd.print("s  ");
}
