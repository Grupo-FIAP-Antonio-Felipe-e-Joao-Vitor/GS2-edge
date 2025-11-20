const ctx = document.getElementById('grafico_ruido').getContext('2d');
const grafico = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [],
    datasets: [
      {
        label: 'Nível ruído',
        data: [],
        borderColor: 'rgba(75, 192, 192, 1)',
        backgroundColor: 'rgba(75, 192, 192, 0.2)',
        borderWidth: 2,
        tension: 0.3
      }
    ]
  },
  options: {
    responsive: true,
    plugins: {
      legend: {
        labels: { color: 'white' },
        font: { size: 8 }
      },
    },
    scales: {
      x: {
        ticks: { color: 'white' }
      },
      y: {
        ticks: { color: 'white' },
        beginAtZero: true
      }
    }
  }
});

async function atualizar_infos() {
  const res = await fetch('/dados');
  const data = await res.json();
  
  // Verifica se o usuário está em um dispositivo móvel
  const isMobile = window.innerWidth < 768;

  // Formata as datas
  grafico.data.labels = data.timestamp.map(ts => {
    const date = new Date(ts);
    return isMobile
      ? date.toLocaleTimeString('pt-BR', { hour12: false }) // mostra só HH:MM:SS
      : date.toLocaleString('pt-BR'); // mostra data completa
  });

  grafico.data.datasets[0].data = data.ruido
  grafico.update();

  document.getElementById("texto_presenca").innerText = data.presenca === 1 ? "Sala Ocupada" : "Sala Livre";
}

atualizar_infos();
setInterval(atualizar_infos, 5000);