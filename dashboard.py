import requests
from flask import Flask, render_template, jsonify

# Inicializa a aplicação Flask
app = Flask(__name__)

# ---------------- Configurações do FIWARE ----------------
IP_FIWARE = "20.46.254.134"     # IP do servidor FIWARE
DEVICE = "Sensor"               # Tipo da entidade (definido no IoT Agent)
DEVICE_ID = "001"               # ID do dispositivo (usado no Orion Context Broker)

# URLs base para acessar os dados de ruído (STH Comet) e presença (Orion)
URL_BASE_RUIDO = f"http://{IP_FIWARE}:8666/STH/v1/contextEntities/type/{DEVICE}/id/urn:ngsi-ld:{DEVICE}:{DEVICE_ID}/attributes"
URL_BASE_PRESENCA = f"http://{IP_FIWARE}:1026/v2/entities/urn:ngsi-ld:{DEVICE}:{DEVICE_ID}/attrs"

# URLs específicas para cada atributo de interesse
URL_RUIDO = f"{URL_BASE_RUIDO}/noise?lastN=30"  # Últimos 30 registros de ruído

URL_PRESENCA = f"{URL_BASE_PRESENCA}/presence"  # Atributo de presença

# Cabeçalhos exigidos pelo FIWARE (service e servicepath)
headers = {
    "fiware-service": "smart",
    "fiware-servicepath": "/"
}

# ---------------- Funções auxiliares ----------------
def pegaDados(url):
    """
    Faz uma requisição HTTP GET para o endpoint informado e retorna o JSON.
    Retorna [] em caso de erro de conexão ou resposta inválida.
    """
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        data = response.json()
        try:
            # Retorna os dados diretamente se forem válidos
            return data
        except KeyError as error:
            print(f"KeyError ao acessar {url}: {error}")
            return []
    else:
        # Caso o FIWARE não responda corretamente
        print(f"Erro ao acessar {url}: Código {response.status_code}")
        return []

# ---------------- Rotas Flask ----------------

@app.route("/dados")
def dados():
    """
    Endpoint que retorna dados em formato JSON.
    - Busca os scores dos times A e B diretamente do Orion Context Broker
    - Busca os níveis de engajamento (histórico) via STH Comet
    - Retorna tudo estruturado em JSON para uso em gráficos/dashboards
    """

    # Requisições aos endpoints do FIWARE
    data_presenca = pegaDados(URL_PRESENCA)

    # Extrai histórico de ruido das salas (últimos 30 registros)
    data_ruido = pegaDados(URL_RUIDO)['contextResponses'][0]['contextElement']['attributes'][0]['values']

    # Extrai valores atuais de presença
    presenca = data_presenca["value"]

    # Converte as listas de ruido em arrays numéricos
    ruido = [float(entry.get('attrValue', 0)) for entry in data_ruido]

    # Extrai timestamps de cada leitura (para gráficos de linha)
    timestamp = [entry.get('recvTime') for entry in data_ruido]

    # Retorna um JSON estruturado
    return jsonify({
        "timestamp": timestamp,
        "presenca": presenca,
        "ruido": ruido
    })

@app.route("/")
def home():
    """
    Página inicial da aplicação Flask.
    Renderiza o arquivo HTML 'index.html' localizado na pasta 'templates'.
    """
    return render_template("index.html")

# ---------------- Execução do servidor ----------------
if __name__ == '__main__':
    # Inicia o servidor Flask em modo debug, acessível em qualquer IP da rede
    app.run(
        debug=True,
        host="0.0.0.0",  # Permite acesso externo (outros dispositivos da rede)
        port=5000        # Porta de execução
    )