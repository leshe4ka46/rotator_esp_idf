<template>
  <div>
    <div v-if="is_admin == 0">
      AUTH NEDED
    </div>
    <div v-if="is_admin == 1">
      <!--<v-select :items="list" :appendIcon="iconarrow" v-model="selectedOption" style="width:30%;"></v-select>-->
      <v-layout text-xs-center wrap>
        <v-card max-width="300px" flat class="bcg">
          <v-container>
            <p class="text-center ">Углы</p>
            <v-text-field label="Азимут" v-model="azimut" type="number" @keyup.enter.exact="sendangles" />
            <v-text-field label="Элевация" v-model="elevation" type="number" @keyup.enter.exact="sendangles" />
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
            <v-btn tile color="success" @click="send_coords">Отправить &nbsp;<MdiSvg>{{ mdiSend }}</MdiSvg>
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
    </div>
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
      home_lat: 0,
      home_long: 0,
      home_height: 0,
      sat_lat: 0,
      sat_long: 0,
      sat_height: 0,
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
          azimut: Number(this.azimut) * 100,
          elevation: Number(this.elevation) * 100
        })
        .catch(error => {
          console.log(error)
        })
    },
    send_coords: function () {
      this.$ajax
        .post('/api/v1/data/set/coords', {
          key: localStorage.getItem('rotator_client_id'),
          sat_lat: Number(this.sat_lat) * 100,
          sat_long: Number(this.sat_long) * 100,
          sat_height: Number(this.sat_height) * 100
        })
        .catch(error => {
          console.log(error)
        })
    },
    send_hoome_coords: function () {
      this.$ajax
        .post('/api/v1/data/set/home_coords', {
          key: localStorage.getItem('rotator_client_id'),
          home_lat: Number(this.home_lat) * 100,
          home_long: Number(this.home_long) * 100,
          home_height: Number(this.home_height) * 100
        })
        .catch(error => {
          console.log(error)
        })
    }
  }
}
</script>

<style>
.bcg{
  background-color:#fafafa !important
}
</style>