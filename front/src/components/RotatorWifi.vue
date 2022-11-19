<template>
  <!--<div>
    <b-button type="is-info" @click="startscan">Scan</b-button>
    <nav class="panel is-info">

<p class="panel-heading is-size-6" v-show="networks.length == 0">
      Выполняется сканирование WI-FI сетей
    </p>
      <p class="panel-heading is-size-6" v-show="networks.length > 0">
        Доступные сети
      </p>
      <a class="panel-block is-active" v-for="network in networks" :key="network.ssid" @click="prompt(network)">
        <span class="panel-icon">
          <i class="fas fa-book" aria-hidden="true"></i>
        </span>
        {{ network.ssid }}
      </a>
    </nav>
  </div>-->
  <div>beta</div>

</template>

<script>

import axios from 'axios'
export default {
  name: 'RotatorWifi',
  data () {
    return {
      networks: []
    }
  },
  created () { /*this.startscan() */},
  methods: {
    prompt (network) {
      if (network.open) {
        this.$buefy.dialog.confirm({
          message: `Подключиться к сети ${network.ssid}?`,
          cancelText: 'Отмена',
          confirmText: 'Подключиться',
          type: 'is-info',
          size: 'is-small',
          onConfirm: () => {
            const connection = { ssid: network.ssid, password: '' }
            this.saveNetwork(connection)
          }
        })
      } else {
        this.$buefy.dialog.prompt({
          message: `Подключение к сети ${network.ssid} <br> Пароль:`,
          type: 'is-info',
          size: 'is-small',
          confirmText: 'Подключиться',
          cancelText: 'Отмена',
          inputAttrs: {
            placeholder: ''
          },
          trapFocus: true,
          onConfirm: (password) => {
            // this.$buefy.toast.open(`Вы подключены к сети: ${value}`);
            const connection = { ssid: network.ssid, password: password }
            this.saveNetwork(connection)
          }
        })
      }
    },
    saveNetwork (connection) {
      // eslint-disable-next-line
      console.log(connection);
      axios.get(`api/connect?ssid=${connection.ssid}&password=${connection.password}`)
      this.$buefy.toast.open('Ок')
    },
    startscan () {
      axios.get('api/scanwifi').then((response) => {
        // eslint-disable-next-line
        console.log(response.data)
        return response.data
      }).then((data) => {
        this.networks = data.networks
      })
    }
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
