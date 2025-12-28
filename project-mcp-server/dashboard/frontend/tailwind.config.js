/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        ombra: {
          dark: '#0a0a0f',
          darker: '#050508',
          accent: '#00ff9d',
          warning: '#ff9d00',
          danger: '#ff3d3d',
          muted: '#4a4a5a',
        }
      }
    },
  },
  plugins: [],
}
