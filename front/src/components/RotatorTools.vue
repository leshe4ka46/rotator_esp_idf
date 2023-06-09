<template>
  <v-container>
    <div>
      <label class="switch" >
        <input type="checkbox" v-model="dorotate" @click="do_rotate_switch" id="sw1">
        <span class="slider round"> </span>
      </label>
      <label for="sw1" > Доворот до точной остановки</label>
    </div>
    <br>
    <v-sheet color="white">
      <v-text-field label="Азимут" v-model="azimut" type="number" @keyup.enter.exact="sendangles" />
      <v-text-field label="Элевация" v-model="elevation" type="number" @keyup.enter.exact="sendangles" />
      <v-btn tile color="success" @click="sendangles">
        Отправить
      </v-btn>
    </v-sheet>

    <!--
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
        <a class="overflow-auto" href="https://play.google.com/store/apps/details?id=com.espressif.provsoftap">Скачать (Android/GP)</a>
        <br>
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
    <br>-->
    <br>
    <p>
      <a class="overflow-auto" href="RotatorApp.apk" v-if="w_iw<1300">Скачать приложение (Android)</a>
    </p>

    <v-snackbar
      v-model="snackbar"
      :timeout="2000">
      Сохранено
    </v-snackbar>
  </v-container>
</template>

<script>
import { bus } from '@/event-bus'
export default {
  name: 'RotatorTools',
  data() {
    return {
      snackbar:false,
      azimut:0,
      elevation:0,
      data:[{name:"AP",value:0},{name:"STA",value:1}],
      val:0,
      dorotate:true,
      w_iw:0
    }
  },
  created() {
    this.w_iw =window.innerWidth
    bus.$on('dorotate_enabled',(val)=>{this.dorotate=(val==1)?true:false})
  },
  methods: {
    do_rotate_switch(){
      this.$ajax
        .post('/api/v1/data/set/dorotate', {
          key: localStorage.getItem('rotator_client_id'),
          value: (this.dorotate==true)?0:1
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

<style>
.switch {
  position: relative;
  display: inline-block;
  width: 40px;
  height: 23px;
}

/* Hide default HTML checkbox */
.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

/* The slider */
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
}

.slider:before {
  position: absolute;
  content: "";
  height: 17px;
  width: 17px;
  left: 3px;
  bottom: 3px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}

input:checked + .slider {
  background-color: #2196F3;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  -webkit-transform: translateX(17px);
  -ms-transform: translateX(17px);
  transform: translateX(17px);
}

/* Rounded sliders */
.slider.round {
  border-radius: 34px;
}

.slider.round:before {
  border-radius: 50%;
}

</style>
