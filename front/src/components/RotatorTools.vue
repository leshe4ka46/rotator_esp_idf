<template>
  <v-container style="overflow: scroll;">
    <v-sheet color="white">
      <v-text-field label="Азимут" v-model="azimut" type="number" @keyup.enter.exact="sendangles" />
      <v-text-field label="Элевация" v-model="elevation" type="number" @keyup.enter.exact="sendangles" />
      <v-btn tile color="success" @click="sendangles">
        Отправить
      </v-btn>
    </v-sheet>
    <br>
    <p>Подключение</p>
    <v-radio-group v-model="val" mandatory>
      <label v-for="(n,index) in data" :key="index" style="font-size:1.25em;">
        <input type="radio" :value="n.value" v-model="val">
        {{n.name}}
      </label>
    </v-radio-group>
    <div v-if="val=='1'">Для подключения надо установить приложение ESP SoftAP Provisioning. В uart0 будет qr код для подключения. Необходим терминал с поддержкой unicode символов. Есть возможность ручного подключения в том же приложении
      <br>
      <p class="text-wrap">
        <!--<a class="overflow-auto" href="https://play.google.com/store/apps/details?id=com.espressif.provsoftap">Скачать (Android/GP)</a>
        <br>-->
        <a class="overflow-auto" href="market://details?id=com.espressif.provsoftap">Скачать (Android/GP)</a>
        <br>
        <a class="overflow-auto" href="https://apps.apple.com/us/app/esp-softap-provisioning/id1474040630">Скачать (iOS)</a>
        <br>
        <a class="overflow-auto" href="https://files.sporadic.ru/f/bba1f4984c084bb8a61a/?dl=1">Скачать (Android/files.sporadic)</a>
        <br>
        <a class="overflow-auto" href="ESP_SoftAP_Prov_2_1_0.apk">Скачать локально</a>
      </p>
    </div>
    <v-btn tile color="success" @click="save_wireless_mode">Сохранить</v-btn>
    <br>

    <v-btn depressed color="primary" @click="zero_as5600">
      Сбросить значение датчика углов
    </v-btn>
    <v-snackbar
      v-model="snackbar"
      :timeout="2000">
      Сохранено
    </v-snackbar>
  </v-container>
</template>

<script>
export default {
  name: 'RotatorTools',
  data() {
    return {
      snackbar:false,
      azimut:0,
      elevation:0,
      data:[{name:"AP",value:0},{name:"STA",value:1}],
      val:0
    }
  },

  methods: {
    zero_as5600() {
      this.$ajax
        .post('/api/v1/reset/as5600/', {
          key: localStorage.getItem('rotator_client_id')
        })
    },
    sendangles() {
      this.$ajax
        .post('/api/v1/data/set/currentangles', {
          key: localStorage.getItem('rotator_client_id'),
          azimut: Number(this.azimut),
          elevation: Number(this.elevation)
        }).then(() => {this.snackbar=true;})
    },
    save_wireless_mode(){
      this.$ajax
        .post('/api/v1/data/set/wifi_mode', {
          key: localStorage.getItem('rotator_client_id'),
          mode: this.val
        })
        .then(() => {this.snackbar=true;})
        .catch(error => {
          console.log(error)
        })
    }
  }
}
</script>
