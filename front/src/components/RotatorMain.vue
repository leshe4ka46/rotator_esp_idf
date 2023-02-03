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
              <span class="grey--text">Напряжение на батарее</span>
              <br>
              <h3><span class="black--text">{{ setted_azimut }}&deg; {{ setted_elevation }}&deg;</span></h3>
              <span class="grey--text">Заданные значения</span>
              <br>
              <span class="red--text" v-if="delta_enabled">Задан относительный угол</span>
              <br v-if="delta_enabled">
            </div>
          </v-card-title>
        </v-card>
      </v-flex>
    </v-layout>

    <v-snackbar v-model="alert_axes" :timeout="0" color="red accent-2">
      Мотор не доехал до конца: {{ motors }}

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
      motors: "",
      alert_x: false,
      alert_y: false,
      delta_enabled:0,
      is_ready:1,
    }
  },
  watch:{
    is_ready(val){
      bus.$emit('is_ready',val)
    }
  },
  created() {
    setInterval(() => {
      this.$ajax.get(`/api/v1/data/get/angles`).then((response) => {
        this.battery = response.data.voltage.toFixed(2);
        this.azimut = response.data.azimut.toFixed(3);
        this.elevation = response.data.elevation.toFixed(2);
        this.setted_azimut = response.data.setted_azimut.toFixed(3);
        this.setted_elevation = response.data.setted_elevation.toFixed(3);
        this.delta_enabled=response.data.delta_enabled;
        this.is_ready=response.data.is_ready;
        this.check_angles();
      });
    }, 1000);
  },

  methods: {
    zeroaxes() {
      this.$ajax
        .post('/api/v1/data/set/nullX', {
          key: localStorage.getItem('rotator_client_id')
        })
    },
    check_angles() {
      //bus.$emit('get_data_angles')
      if (Math.abs(this.prev_azimut - this.azimut) < 1) {
        if (Math.abs(this.azimut - this.setted_azimut) > 2) {
          this.alert_x = true;
        }
        else {
          this.alert_x = false;
        }
      }

      if (Math.abs(this.prev_elevation - this.elevation) < 1) {
        if (Math.abs(this.elevation - this.setted_elevation) > 2) {
          this.alert_y = true;
        }
        else {
          this.alert_y = false;
        }
      }
      this.prev_azimut = this.azimut
      this.prev_elevation = this.elevation
      if (this.alert_y + this.alert_x > 0) {
        this.alert_axes = true;
        this.motors = ""
        if (this.alert_x) {
          this.motors += "X"
        }
        if (this.alert_x & this.alert_y) {
          this.motors += " и "
        }
        if (this.alert_y) {
          this.motors += "Y"
        }
      }
      else{
        this.alert_axes = false;
      }

    }
  }
}
</script>
