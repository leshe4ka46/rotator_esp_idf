<template>
  <div>
    <v-layout text-xs-center wrap>
      <v-card max-width="300px" flat class="bcg">
        <v-container>
          <p class="text-center ">Углы</p>
          <v-text-field label="Азимут" v-model="azimut" type="number" v-on:keyup.enter="onEnter" />
          <v-text-field label="Элевация" v-model="elevation" type="number" v-on:keyup.enter="onEnter" />
        </v-container>
        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn tile color="success" @click="send_angles">Отправить &nbsp;<MdiSvg>{{ mdiSend }}</MdiSvg>
          </v-btn>
        </v-card-actions>
      </v-card>
      <div class="d-inline pa-2 bcg">
      </div>
      <v-card max-width="300px" flat class="bcg">
        <p class="text-center ">Координаты спутника</p>
        <v-container>
          <v-text-field label="Широта" v-model="sat_lat" type="number" />
          <v-text-field label="Долгота" v-model="sat_long" type="number" />
          <v-text-field label="Высота" v-model="sat_height" type="number" />
        </v-container>
        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn tile color="success" @click="send_sat_coords">Отправить &nbsp;<MdiSvg>{{ mdiSend }}</MdiSvg>
          </v-btn>
        </v-card-actions>
      </v-card>
      <div class="d-inline pa-2 bcg">
      </div>
      <v-card max-width="300px" flat class="bcg">
        <p class="text-center ">
          Координаты поворотки
        </p>
        <v-container>
          <v-text-field label="Широта" v-model="home_lat" type="number" />
          <v-text-field label="Долгота" v-model="home_long" type="number" />
          <v-text-field label="Высота" v-model="home_height" type="number" />
        </v-container>
        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn tile color="success" @click="send_home_coords">Отправить &nbsp;<MdiSvg>{{ mdiSend }}</MdiSvg>
          </v-btn>
        </v-card-actions>
      </v-card>
    </v-layout>
    <v-snackbar v-model="snackbar" :timeout="2000">
      Сохранено
    </v-snackbar>
  </div>
</template>

<script>
import { mdiSend, mdiArrowDownDropCircleOutline } from '@mdi/js'
import { bus } from '@/event-bus'
export default {
  name: 'RotatorDataset',
  data() {
    return {
      mdiSend,
      mdiArrowDownDropCircleOutline,
      iconarrow: '',
      is_admin: 0,
      azimut: 0,
      elevation: 0,
      home_lat: 51.738396,
      home_long: 36.148329,
      home_height: 161,
      sat_lat: 51.750014,
      sat_long: 36.168436,
      sat_height: 2000,
      snackbar: false,
      msg: '0',
      /*list: ['GPS_CALC', 'UART', 'GPS+UART'],
      data: [{ id: 0, name: 'GPS_CALC' }, { id: 1, name: 'UART' }, { id: 2, name: 'GPS+UART' }],
      selectedOption: 1*/
    }
  },
  /*watch: {
    selectedOption (value) {
      this.$ajax.get(`api/mode?mode=${value}&key=${localStorage.getItem('rotator_client_id')}`)
    }
  },*/
  created() {
    bus.$on('is_admin', (val) => { this.is_admin = val })
    bus.$emit('get_admin')
  },
  methods: {
    send_angles: function () {
      bus.$emit('save_data_angles', { azimut: Number(this.azimut), elevation: Number(this.elevation) })
      this.$ajax
        .post('/api/v1/data/set/angles', {
          key: localStorage.getItem('rotator_client_id'),
          azimut: Number(this.azimut),
          elevation: Number(this.elevation)
        })
        .then(() => { this.snackbar = true; })
        .catch(error => {
          console.log(error)
        })
    },
    send_sat_coords: function () {
      this.$ajax
        .post('/api/v1/data/set/satgps', {
          key: localStorage.getItem('rotator_client_id'),
          lat: Number(this.sat_lat),
          lon: Number(this.sat_long),
          height: Number(this.sat_height)
        })
        .then(() => { this.snackbar = true; })
        .catch(error => {
          console.log(error)
        })
    },
    send_home_coords: function () {
      this.$ajax
        .post('/api/v1/data/set/homegps', {
          key: localStorage.getItem('rotator_client_id'),
          lat: Number(this.home_lat),
          lon: Number(this.home_long),
          height: Number(this.home_height)
        })
        .then(() => { this.snackbar = true; })
        .catch(error => {
          console.log(error)
        })
    },
    onEnter: function() {
       this.send_angles()
    }

  }
}
</script>