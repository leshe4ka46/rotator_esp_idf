<template>
  <v-container>

    <v-btn depressed color="primary" @click="zeroaxesX">
      Сбросить значение граусов Х
    </v-btn>
    <v-btn depressed color="primary" @click="zeroaxesY">
      Сбросить значение граусов Y
    </v-btn>
    <v-sheet color="white">
      <v-text-field label="Азимут" v-model="azimut" type="number" @keyup.enter.exact="sendangles" />
      <v-text-field label="Элевация" v-model="elevation" type="number" @keyup.enter.exact="sendangles" />
      <v-btn tile color="success" @click="sendangles">
        Отправить
      </v-btn>
    </v-sheet>


    <br><br>
    <p>Подключение</p>
    <v-radio-group v-model="val" mandatory>
      <label v-for="(n,index) in data" :key="index" style="font-size:1.25em;">
        <input type="radio" :value="n.value" v-model="val">
        {{n.name}}
      </label>


    </v-radio-group>

    <div v-if="val=='1'">Для подключения надо установить приложение ESP SoftAP Provisioning. В uart0 будет qr код для подключения.
      <br>
      <p class="text-wrap">
        <a class="overflow-auto" href="https://play.google.com/store/apps/details?id=com.espressif.provsoftap">https://play.google.com/store/apps/details?id=com.espressif.provsoftap</a>
      </p>
    </div>
    <v-btn tile color="success" @click="save_wireless_mode">Сохранить</v-btn>
  </v-container>
</template>

<script>
export default {
  name: 'RotatorTools',
  data() {
    return {
      azimut:0,
      elevation:0,
      data:[{name:"AP",value:0},{name:"STA",value:1}],
      val:0
    }
  },

  methods: {
    zeroaxesX() {
      this.$ajax
        .post('/api/v1/data/set/nullX', {
          key: localStorage.getItem('rotator_client_id')
        })
    },
    zeroaxesY() {
      this.$ajax
        .post('/api/v1/data/set/nullY', {
          key: localStorage.getItem('rotator_client_id')
        })
    },
    sendangles() {
      this.$ajax
        .post('/api/v1/data/set/currentangles', {
          key: localStorage.getItem('rotator_client_id'),
          azimut: Number(this.azimut)*100,
          elevation: Number(this.elevation)*100
        })
    },
    save_wireless_mode(){
      this.$ajax
        .post('/api/v1/data/set/wifi_mode', {
          key: localStorage.getItem('rotator_client_id'),
          mode: this.val
        })
        .catch(error => {
          console.log(error)
        })
    }
  }
}
</script>
