<template>
  <div>
    <div v-if="is_admin==0">
      AUTH NEDED
    </div>
    <div v-if="is_admin==1">
      <!--<v-select :items="list" :appendIcon="iconarrow" v-model="selectedOption" style="width:30%;"></v-select>-->
      <v-layout text-xs-center wrap>
        <v-card width="300px" flat>
          <v-container>
            <p class="text-center ">
              Углы
            </p>
            <v-text-field label="Азимут" v-model="azimut" type="number" @keyup.enter.exact="sendangles"/>
            <v-text-field label="Элевация" v-model="elevation" type="number"  @keyup.enter.exact="sendangles"/>
          </v-container>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn tile color="success" @click="sendpostdata">
              Отправить
              &nbsp;
              <MdiSvg>{{mdiSend }}</MdiSvg>
            </v-btn>
          </v-card-actions>
        </v-card>
        <div class="d-inline pa-2 white">
        </div>
        <v-card width="300px" flat>
          <p class="text-center ">
            Координаты спутника
          </p>
          <v-container>
            <v-text-field label="Широта" v-model="sat_lat" type="number" />
            <v-text-field label="Долгота" v-model="sat_long" type="number" />
            <v-text-field label="Высота" v-model="sat_height" type="number" />
          </v-container>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn tile color="success">
              Отправить
              &nbsp;
              <MdiSvg>{{mdiSend }}</MdiSvg>
            </v-btn>
          </v-card-actions>
        </v-card>
        <div class="d-inline pa-2 white">
        </div>
        <v-card width="300px" flat>
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
            <v-btn tile color="success">

              Отправить
              &nbsp;
              <MdiSvg>{{mdiSend }}</MdiSvg>
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-layout>
    </div>
  </div>
</template>

<script>
import { mdiSend, mdiArrowDownDropCircleOutline } from '@mdi/js'
// eslint-disable-next-line
import { bus } from '@/event-bus'
export default {
  name: 'RotatorDataset',
  data () {
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
      list: ['GPS_CALC', 'UART', 'GPS+UART'],
      data: [{ id: 0, name: 'GPS_CALC' }, { id: 1, name: 'UART' }, { id: 2, name: 'GPS+UART' }],
      selectedOption: 1
    }
  },
  watch: {
    selectedOption (value) {
      this.$ajax.get(`api/mode?mode=${value}&key=${localStorage.getItem('rotator_client_id')}`)
    }
  },
  created () {
    bus.$on('is_admin', (val) => { this.is_admin = val })
    bus.$emit('get_admin')
  },
  methods: {
    sendangles(){
      this.sendpostdata();
    },
    /*angles () {
      bus.$emit('send_ws', 'ANGLES;' + this.azimut.replace(',', '.') + ';' + this.elevation.replace(',', '.') + ';')
    },
    coords () {
      bus.$emit('send_ws', 'COORDS;' + this.home_lat.replace(',', '.') + ';' + this.home_long.replace(',', '.') + ';' + this.home_height.replace(',', '.') + ';' + this.sat_lat.replace(',', '.') + ';' + this.sat_long.replace(',', '.') + ';' + this.sat_height.replace(',', '.') + ';')
    },*/
    sendpostdata: function () {
      bus.$emit('save_data_angles',{azimut: Number(this.azimut),elevation: Number(this.elevation)})
      this.$ajax
        .post('/api/v1/data/set/angles', {
          key: localStorage.getItem('rotator_client_id'),
          azimut: Number(this.azimut)*100,
          elevation: Number(this.elevation)*100
        })
        .then(data => {
          console.log(data)
        })
        .catch(error => {
          console.log(error)
        })
    }
  }
}
</script>
