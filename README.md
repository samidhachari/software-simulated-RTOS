# 🧠 Software-Simulated RTOS Visualizer

A fully software-simulated Real-Time Operating System (RTOS) written in **C** with a dynamic **React + D3.js** web visualizer.

This project showcases:
- Priority-based scheduling
- Interrupt handling
- Task delays & event waits
- Mutexes and resource locking
- Power states (sleep, wake)
- Interactive Gantt chart for task timelines

## 🌐 Live Demo

👉 [Try it Live](https://samidhachari.github.io/software-simulated-RTOS/) 
Built using Vite + React + D3.js

## 📂 Features

- 🧵 Task simulation with RUNNING, READY, BLOCKED, etc.
- ⏱️ Tick-based timeline with animation controls (Play / Pause / Speed)
- 🎨 Visual task filtering by state (dropdown)
- 📉 CSV log ingestion (50+ data points)
- 💡 Great for embedded/Firmware/RTOS learning

## 🛠 Tech Stack

| Area         | Tools Used                     |
|--------------|--------------------------------|
| Backend Sim  | C (RTOS scheduler simulator)   |
| Frontend     | React + D3.js + Vite           |
| Styling      | CSS (custom + responsive)      |
| Deployment   | GitHub Pages                   |

## 📁 File Structure

.
├── public/
│ └── task_log.csv
├── src/
│ ├── App.jsx
│ ├── RTOSVisualizer.jsx
├── vite.config.js

## 🚀 How to Run Locally

```bash
git clone https://github.com/samidhachari/software-simulated-RTOS.git
cd software-simulated-RTOS
npm install
npm run dev
Open http://localhost:5173 in your browser.





# React + Vite

This template provides a minimal setup to get React working in Vite with HMR and some ESLint rules.

Currently, two official plugins are available:

- [@vitejs/plugin-react](https://github.com/vitejs/vite-plugin-react/blob/main/packages/plugin-react) uses [Babel](https://babeljs.io/) for Fast Refresh
- [@vitejs/plugin-react-swc](https://github.com/vitejs/vite-plugin-react/blob/main/packages/plugin-react-swc) uses [SWC](https://swc.rs/) for Fast Refresh

## Expanding the ESLint configuration

If you are developing a production application, we recommend using TypeScript with type-aware lint rules enabled. Check out the [TS template](https://github.com/vitejs/vite/tree/main/packages/create-vite/template-react-ts) for information on how to integrate TypeScript and [`typescript-eslint`](https://typescript-eslint.io) in your project.

