<template>
  <div class="container">
    <v-layout text-xs-center wrap>
      <v-flex xs12 sm6 offset-sm3>
        <v-card>
          <v-card-title primary-title>
            <div class="ma-auto">
              <h1><span class="black--text">{{ azimut }}&deg;</span></h1>
              <span class="grey--text">Азимут</span>
              <br>
              <h1><span class="black--text">{{ elevation }}&deg;</span></h1>
              <span class="grey--text">Элевация</span>
              <br>
              <h1><span class="black--text">{{ battery }} B</span></h1>
              <span class="grey--text">Напряжение батареи</span>
              <br>
              <h3><span class="black--text">{{ setted_azimut }}&deg; {{ setted_elevation }}&deg;</span></h3>
              <span class="grey--text">Заданные значения</span>
              <br>

            </div>
          </v-card-title>
        </v-card>
      </v-flex>
    </v-layout>

    <v-snackbar v-model="alert_axes" :timeout="0"  color="red accent-2">
      Мотор, походу, не доехал до конца
      <!--<template>
        <v-btn color="red" text @click="alert_axes = false; alerting_done = true;">
          Закрыть
        </v-btn>
      </template>-->
    </v-snackbar>
  </div>
</template>

<script>
import { bus } from '@/event-bus'

export default {
  name: 'RotatorMain',
  data() {
    return {
      azimut: 0,
      elevation: 0,
      battery: 0,
      ws: null,
      prev_azimut: 0,
      setted_azimut: 0,
      prev_elevation: 0,
      setted_elevation: 0,
      alert_axes: false,
      alerting_done: false,
      alert_times: 0,
    }
  },
  created() {
    /*bus.$on('return_data_angles', (data) => {
      this.setted_azimut = data.azimut
      this.setted_elevation = data.elevation
    })*/
    /*this.ws = new WebSocket(`ws://${window.location.hostname}/ws`) // `ws://localhost:2000/`
    this.ws.onopen = function () {
      // eslint-disable-next-line
      console.log('WS подключен')
    }
    this.ws.onclose = function (eventclose) {
      // eslint-disable-next-line
      console.log('Cоеденение закрыто: ')
      // eslint-disable-next-line
      console.log(eventclose)
    }
    this.ws.onmessage = function (msg) {
      var response = JSON.parse(msg.data)
      this.battery = response.voltage.toFixed(2); this.azimut = response.azimut.toFixed(2); this.elevation = response.elevation.toFixed(2)
    }.bind(this)
    bus.$on('send_ws', (mess) => { this.ws.send(mess) })*/
    setInterval(() => {
      this.$ajax.get(`/api/v1/data/get/angles`).then((response) => {
        this.battery = response.data.voltage.toFixed(2);
        this.azimut = response.data.azimut.toFixed(3);
        this.elevation = response.data.elevation.toFixed(2);
        this.setted_azimut = response.data.setted_azimut.toFixed(3);
        this.setted_elevation = response.data.setted_elevation.toFixed(3);
      });
    }, 1000);
    setInterval(() => { this.check_angles() }, 1000);
  },

  methods: {
    zeroaxes() {
      this.$ajax
        .post('/api/v1/data/set/nullX', {
          key: localStorage.getItem('rotator_client_id')
        })
    },
    check_angles() {
      bus.$emit('get_data_angles')

      if (Math.abs(this.prev_azimut - this.azimut) < 1) {
        if (Math.abs(this.azimut - this.setted_azimut) > 2) {
          if (/*this.alerting_done == false && */this.alert_times != 0) {
            this.alert_axes = true;
          }
          this.alert_times += 1
        }
        else {
          this.alert_times = 0
          this.alerting_done = false;
          this.alert_axes = false;
        }
      }

      this.prev_azimut = this.azimut

    }
  }
}
</script>
