#include <WiFi.h>
#include <WebServer.h>

// =======================================================
// CONFIGURACIÓN EN MODO ACCESS POINT (Sin Router)
// =======================================================
// La ESP32 creará su propia red Wi-Fi para que te conectes directo
const char* ap_ssid     = "FLIA. LOPEZ";
const char* ap_password = "carlos1975";

// Credenciales para el inicio de sesión web
const char* www_username = "admin";
const char* www_password = "1234";

// Servidor Web en el puerto 80
WebServer server(80);

// =======================================================
// Variables simuladas para probar la interfaz sin sensores
bool sistemaActivo = false;
unsigned long tiempoInicio = 0;
unsigned long tiempoTranscurrido = 0;
float temperaturaSimulada = 25.5;

// =======================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Prueba ESP32 - Sin Sensores</title>
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
    * { box-sizing: border-box; font-family: 'Segoe UI', system-ui, sans-serif; }
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
      background: #0f172a; color: #fff; font-size: 1rem; outline: none;
    }
    .btn-login {
      width: 100%; padding: 0.8rem; border-radius: 0.5rem; border: none; background: #0284c7;
      color: white; font-size: 1rem; font-weight: bold; cursor: pointer;
    }
    .btn-login:hover { background: #0369a1; }
    .error-msg { color: var(--danger); font-size: 0.85rem; margin-top: 0.8rem; text-align: center; display: none; }

    .dashboard { display: none; width: 100%; max-width: 500px; }
    .navbar {
      display: flex; justify-content: space-between; align-items: center; width: 100%;
      background: var(--card-bg); padding: 1rem 1.5rem; border-radius: 0.75rem;
      border: 1px solid var(--border); margin-bottom: 1.5rem;
    }
    .navbar h1 { color: var(--accent); font-size: 1.1rem; margin: 0; }
    .btn-logout {
      background: #334155; color: #f1f5f9; border: none; padding: 0.5rem 1rem;
      border-radius: 0.4rem; cursor: pointer; font-size: 0.85rem;
    }
    .cards-container { display: flex; flex-direction: column; gap: 1.2rem; width: 100%; }
    .card {
      background: var(--card-bg); padding: 1.5rem; border-radius: 0.8rem;
      border: 1px solid var(--border); text-align: center;
    }
    .card h3 { margin: 0 0 0.5rem 0; font-size: 1rem; color: #94a3b8; font-weight: normal; }
    .val-large { font-size: 2.3rem; font-weight: bold; color: var(--warning); margin-top: 0.5rem; }
    .status-badge {
      display: inline-block; padding: 0.5rem 1.2rem; border-radius: 2rem; font-weight: bold; font-size: 1.2rem; margin-top: 0.5rem;
    }
    .status-on { background: rgba(74, 222, 128, 0.15); color: var(--success); border: 1px solid var(--success); }
    .status-off { background: rgba(248, 113, 113, 0.15); color: var(--danger); border: 1px solid var(--danger); }
    .test-btn {
      margin-top: 1rem; background: #059669; color: white; border: none; padding: 0.6rem 1.2rem;
      border-radius: 0.5rem; font-weight: bold; cursor: pointer; font-size: 0.9rem;
    }
    .test-btn:hover { background: #047857; }
  </style>
</head>
<body>

  <div id="login-container" class="login-card">
    <div class="login-header">
      <h2>Prueba ESP32</h2>
      <p>Modo de Verificación sin Hardware</p>
    </div>
    <form id="login-form" onsubmit="event.preventDefault(); doLogin();">
      <div class="form-group">
        <label for="username">Usuario</label>
        <input type="text" id="username" value="admin" required>
      </div>
      <div class="form-group">
        <label for="password">Contraseña</label>
        <input type="password" id="password" value="1234" required>
      </div>
      <button type="submit" class="btn-login">Ingresar al Test</button>
      <div id="login-error" class="error-msg">Credenciales incorrectas</div>
    </form>
  </div>

  <div id="dashboard-container" class="dashboard">
    <div class="navbar">
      <h1>● ESP32 Online (Test)</h1>
      <button class="btn-logout" onclick="doLogout()">Salir</button>
    </div>

    <div class="cards-container">
      <div class="card">
        <h3>Estado del Sistema (Simulado)</h3>
        <div id="status-badge" class="status-badge status-off">○ DETENIDO</div>
        <br>
        <button class="test-btn" onclick="toggleSim()">Simular Inicio / Parada</button>
      </div>
      <div class="card">
        <h3>Temperatura Simulada (PT100)</h3>
        <div id="temp-val" class="val-large">--.- °C</div>
      </div>
      <div class="card">
        <h3>Cronómetro de Prueba</h3>
        <div id="time-val" class="val-large" style="color: var(--accent);">00:00</div>
      </div>
    </div>
  </div>

  <script>
    let pollInterval = null;

    window.addEventListener('DOMContentLoaded', () => {
      if (sessionStorage.getItem('test_auth') === 'true') {
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
          sessionStorage.setItem('test_auth', 'true');
          errEl.style.display = 'none';
          showDashboard();
        } else {
          errEl.style.display = 'block';
        }
      })
      .catch(() => alert('Error de red con la ESP32'));
    }

    function doLogout() {
      sessionStorage.removeItem('test_auth');
      if (pollInterval) clearInterval(pollInterval);
      document.getElementById('dashboard-container').style.display = 'none';
      document.getElementById('login-container').style.display = 'block';
    }

    function showDashboard() {
      document.getElementById('login-container').style.display = 'none';
      document.getElementById('dashboard-container').style.display = 'block';
      updateData();
      if (!pollInterval) {
        pollInterval = setInterval(updateData, 1000);
      }
    }

    function toggleSim() {
      fetch('/toggle', { method: 'POST' })
        .then(() => updateData());
    }

    function updateData() {
      fetch('/datos')
        .then(r => r.json())
        .then(data => {
          document.getElementById('temp-val').innerText = data.temperatura.toFixed(1) + ' °C';

          const statusEl = document.getElementById('status-badge');
          if (data.activo) {
            statusEl.innerText = '● FUNCIONANDO';
            statusEl.className = 'status-badge status-on';
          } else {
            statusEl.innerText = '○ DETENIDO';
            statusEl.className = 'status-badge status-off';
          }

          let totalSecs = Math.floor(data.tiempo / 1000);
          let m = Math.floor(totalSecs / 60);
          let s = totalSecs % 60;
          document.getElementById('time-val').innerText = (m < 10 ? '0' + m : m) + ':' + (s < 10 ? '0' + s : s);
        });
    }
  </script>
</body>
</html>
)rawliteral";

// =======================================================
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleLogin() {
  if (server.hasArg("username") && server.hasArg("password")) {
    if (server.arg("username") == www_username && server.arg("password") == www_password) {
      server.send(200, "application/json", "{\"status\":\"ok\"}");
      return;
    }
  }
  server.send(401, "application/json", "{\"status\":\"unauthorized\"}");
}

void handleToggle() {
  sistemaActivo = !sistemaActivo;
  if (sistemaActivo) {
    tiempoInicio = millis();
  }
  server.send(200, "application/json", "{\"status\":\"toggled\"}");
}

void handleDatos() {
  // Simular ligera variación de temperatura cuando está activo
  if (sistemaActivo) {
    temperaturaSimulada += 0.2;
    if (temperaturaSimulada > 65.0) temperaturaSimulada = 25.5;
    tiempoTranscurrido = millis() - tiempoInicio;
  } else {
    tiempoTranscurrido = 0;
  }

  String json = "{";
  json += "\"temperatura\":" + String(temperaturaSimulada, 1) + ",";
  json += "\"activo\":" + String(sistemaActivo ? "true" : "false") + ",";
  json += "\"tiempo\":" + String(tiempoTranscurrido);
  json += "}";
  server.send(200, "application/json", json);
}

// =======================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\nIniciando ESP32 en Modo de Prueba (Sin Sensores)...");

  // Iniciar red Wi-Fi propia (Access Point)
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Red Wi-Fi creada con éxito.\nNombre de Red: ");
  Serial.println(ap_ssid);
  Serial.print("Dirección IP de acceso: http://");
  Serial.println(IP); // Por defecto será 192.168.4.1

  // Configurar rutas del servidor web
  server.on("/", handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.on("/datos", handleDatos);

  server.begin();
  Serial.println("Servidor web listo para recibir conexiones.");
}

// =======================================================
void loop() {
  server.handleClient();
}
