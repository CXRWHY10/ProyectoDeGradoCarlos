# 🧼 Prototipo Automatizado de Mezclado y Saponificación de Aceite Reciclado

> **Proyecto de Grado** — *Carrera de Informática Industrial*  
> *Diseño e implementación de un prototipo a escala funcional para la automatización de la producción de jabón ecológico.*

![Estado](https://img.shields.io/badge/Estado-Prototipo%20en%20Desarrollo-orange)
![Controlador](https://img.shields.io/badge/PLC-Siemens%20LOGO!%208-blue)
![Software](https://img.shields.io/badge/Software-LOGO!%20Soft%20Comfort-brightgreen)
![Escala](https://img.shields.io/badge/Escala-Banco%20de%20Pruebas-yellow)

---

## 📋 Descripción General del Prototipo

Este repositorio alberga la lógica de automatización, la arquitectura de control y la documentación técnica de un **prototipo a escala de laboratorio** para el proceso de **saponificación**. 

El objetivo principal del proyecto es automatizar la mezcla precisa de tres insumos clave: **aceite vegetal reciclado (AVU)**, **agua destilada** e **hidróxido de sodio (NaOH)**, garantizando un control secuencial seguro y eficiente que transforme residuos de cocina en jabón ecológico.

---

## 🎨 Diseño Visual y Modelo 3D de la Máquina

<div align="center">
  <!-- Puedes reemplazar esta ruta con la imagen de tu render 3D o diseño en CAD -->
  <img src="https://via.placeholder.com/700x380.png?text=Inserte+aqui+el+Render+3D+o+Foto+del+Prototipo" alt="Render 3D del Prototipo" width="650px" />
  <p><i>Figura 1: Vista tridimensional del prototipo (Tanques de reactivos, cámara de mezclado y disposición del motor).</i></p>
</div>

### 📐 Distribución del Sistema (Esquema P&ID / Bloques)

<div align="center">
  <!-- Puedes colocar aquí tu diagrama eléctrico o de tuberías e instrumentación -->
  <img src="https://via.placeholder.com/650x300.png?text=Inserte+aqui+el+Esquema+P%26ID+o+Diagrama+Electrico" alt="Esquema P&ID" width="600px" />
  <p><i>Figura 2: Diagrama de instrumentación y flujo de insumos hacia la cámara principal de agitación.</i></p>
</div>

---

## 🔬 Fundamento del Proceso Químico

El prototipo automatiza la reacción de **saponificación fría/semicaliente**, controlando la adición de la lejía (NaOH + H₂O) al aceite filtrado:

$$\text{Triglicéridos (Aceite Reciclado)} + 3\,\text{NaOH} \xrightarrow{\text{Agitación + Tiempo}} \text{Glicerina} + 3\,\text{Jabón (Sales Sódicas)}$$

### Variables Controladas en el Prototipo:
1. **Relación de Volumen:** Dosificación automatizada de cada tanque mediante temporización y sensores de nivel.
2. **Control de Mezclado:** Agitación constante para asegurar la emulsión y conseguir el "punto de traza".
3. **Secuencia de Seguridad:** Adición controlada de la solución alcalina para mitigar riesgos por reacción exotérmica.

---

## 🛠️ Especificaciones del Hardware y Componentes

| Componente | Elemento Utilizado en el Prototipo | Función Técnica |
| :--- | :--- | :--- |
| **Controlador Principal** | PLC Siemens LOGO! 12/24 RCE (v8) | Ejecución de la lógica secuencial y temporizaciones |
| **Actuador de Mezcla** | Motor DC 12V/24V con acople a paleta | Agitación y homogeneización de la mezcla |
| **Dosificación de Insumos** | Electroválvulas Solenoide / Bombas 12V | Control de apertura y paso de insumos a la cámara |
| **Sensores de Nivel** | Sensores de flotador / ópticos de nivel | Indicación de tanque lleno/vacío (Aceite, Lejía y Mezcla) |
| **Sensor de Temperatura** | RTD PT100 o Sensor Análogo | Monitoreo térmico durante la reacción de la lejía |
| **Fuente de Alimentación** | Fuente Conmutada Industrial 24VDC / 12VDC | Energización de relés, PLC y actuadores |
| **Interfaz de Control** | Pulsadores (NO/NC) + Parada de Emergencia | Control manual/automático y paro de seguridad |

---

## 🧠 Secuencia de Automatización (GRAFCET)

El control del prototipo sigue una estructura lógica secuencial dividida en las siguientes fases:

```mermaid
graph TD
    A[Etapa 0: Estado de Reposo / Espera] -->|Pulsador START + Nivel OK| B[Etapa 1: Llenado de Agua e Hidróxido]
    B -->|Sensor Nivel Lejía Activado| C[Etapa 2: Preparación de Solución Alcalina]
    C -->|Tiempo / Temp. Optima| D[Etapa 3: Inyección de Aceite Reciclado]
    D -->|Nivel Mezclador OK| E[Etapa 4: Activación del Agitador]
    E -->|Tiempo de Saponificación Cumplido| F[Etapa 5: Vaciado del Jabón Líquido/Traza]
    F -->|Tanque Mezclador Vacío| A
