# 🧼 Prototipo Automatizado de Mezclado y Saponificación de Aceite Reciclado

> **Proyecto de Grado** — *Carrera de Informática Industrial*  
> *Diseño e implementación de un prototipo a escala funcional para la automatización de la producción de jabón ecológico.*

---

## 📋 Descripción General del Prototipo

Este repositorio alberga la lógica de automatización, la arquitectura de control y la documentación técnica de un **prototipo a escala de laboratorio** para el proceso de **saponificación**. 

El objetivo principal del proyecto es automatizar la mezcla precisa de tres insumos clave: **aceite vegetal reciclado (AVU)**, **agua destilada** e **hidróxido de sodio (NaOH)**, garantizando un control secuencial seguro y eficiente que transforme residuos de cocina en jabón ecológico.

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
| **Sensor de Temperatura** | RTD PT100 o Sensor Análogo | Monitoreo térmico durante la reacción de la lejía |
| **Fuente de Alimentación** | Fuente Conmutada Industrial 24VDC / 12VDC | Energización de relés, PLC y actuadores |
| **Interfaz de Control** | Pulsadores (NO/NC) + Parada de Emergencia | Control manual/automático y paro de seguridad |

---

## 🧠 Secuencia de Automatización

El control del prototipo sigue una estructura lógica secuencial dividida en las siguientes fases:

```mermaid
graph TD
    A[Etapa 0: Estado de Reposo / Espera] -->|Pulsador START + Nivel OK| B[Etapa 1: Llenado de Agua e Hidróxido]
    B -->|Sensor Nivel Lejía Activado| C[Etapa 2: Preparación de Solución Alcalina]
    C -->|Tiempo / Temp. Optima| D[Etapa 3: Inyección de Aceite Reciclado]
    D -->|Nivel Mezclador OK| E[Etapa 4: Activación del Agitador]
    E -->|Tiempo de Saponificación Cumplido| F[Etapa 5: Vaciado del Jabón Líquido/Traza]
    F -->|Tanque Mezclador Vacío| A
