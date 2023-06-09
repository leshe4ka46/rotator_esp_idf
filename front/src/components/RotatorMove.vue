<template>
  <v-container>
    <v-layout text-xs-center wrap>
      <v-flex xs12 sm6 offset-sm3>
        <h1>
          <span class="black--text">{{ azimutReal }}&deg;</span>&#09;<span class="black--text">{{ azimutCorrected
          }}&deg;</span>
        </h1>
        <span class="grey--text">Азимут</span>
        <br>
        <h1><span class="black--text">{{ elevationReal }}&deg;</span>&#09;<span class="black--text">{{ elevationCorrected
        }}&deg;</span></h1>
        <span class="grey--text">Элевация</span>
        <br>
        <v-btn @click="buttonClick(0)" class="custom-button">
          <MdiSvg size="50">{{ mdiHome }}</MdiSvg>
        </v-btn>
        <br>
        <v-row justify="center" align="center" class="text-center">
          <v-col cols="4" class="text-center">
            <v-btn @click="buttonClick(1)" class="custom-button">
              <MdiSvg size="50">{{ mdiArrowUpThin }}</MdiSvg>
            </v-btn>
          </v-col>
        </v-row>
        <br>
        <v-row justify="center" align="center">
          <v-col cols="4" class="text-center">
            <v-btn @click="buttonClick(2)" class="custom-button">
              <MdiSvg size="50">{{ mdiArrowLeftThin }}</MdiSvg>
            </v-btn>
            <v-btn @click="buttonClick(3)" class="custom-button">
              <MdiSvg size="50">{{ mdiArrowRightThin }}</MdiSvg>
            </v-btn>
          </v-col>
        </v-row>
        <br>
        <v-row justify="center" align="center">
          <v-col cols="4" class="text-center">
            <v-btn @click="buttonClick(4)" class="custom-button">
              <MdiSvg size="50">{{ mdiArrowDownThin }}</MdiSvg>
            </v-btn>
          </v-col>
        </v-row>
        <br>
        <v-btn-toggle mandatory v-model="toggle">
          <v-btn v-for="(distance, index) of moveDistances" :key="index" min-width="40" :value="distance" color="#0288D1"
            :elevation="0" class="white--text ">
            {{ distance }}
          </v-btn>
        </v-btn-toggle>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
import { bus } from '@/event-bus'
import { mdiArrowUpThin, mdiArrowDownThin, mdiArrowLeftThin, mdiArrowRightThin, mdiHome } from '@mdi/js';
export default {
  name: 'RotatorMove',
  props: ["opened"],
  data() {
    return {
      moveDistances: [0.1, 0.25, 1, 5, 10, 25],
      toggle: 1,
      mdiArrowUpThin, mdiArrowDownThin, mdiArrowLeftThin, mdiArrowRightThin, mdiHome,
      safeJoy: null,
      azimutReal: 0,
      azimutCorrected: 0,
      elevationReal: 0,
      elevationCorrected: 0
    }
  },
  methods: {
    buttonClick(val) {
      this.$ajax
        .post('/api/v1/data/set/delta', {
          key: localStorage.getItem('rotator_client_id'),
          axis: (val == 1 || val == 4) ? 1 : 0,
          angle: ((val == 2 || val == 4) ? -1 : 1) * this.toggle,
          reset: (val == 0) * 1
        })
    },

  },
  mounted() {
    bus.$on("safeJoy", (val) => {
      this.safeJoy = val;
    })
    setInterval(() => {
      if (this.opened) {
        this.$ajax.get(`/api/v1/data/get/joyangles`).then((response) => {
          this.azimutReal = response.data.azimut.toFixed(3);
          this.azimutCorrected = (Number(response.data.azimut) + Number(response.data.deltajoyazimut)).toFixed(3);
          this.elevationReal = response.data.elevation.toFixed(3);
          this.elevationCorrected = (Number(response.data.elevation) + Number(response.data.deltajoyelevation)).toFixed(3);
        }).catch(error => { // eslint-disable-next-line
          console.log(error);
        });
      }
    }, 1000);
    document.addEventListener('keyup', (event) => {
      if (this.opened == false && this.safeJoy) {
        return 0
      }
      switch (event.key) {
        case "1":
          this.toggle = 0.1
          break;
        case "2":
          this.toggle = 0.25
          break;
        case "3":
          this.toggle = 1
          break;
        case "4":
          this.toggle = 5
          break;
        case "5":
          this.toggle = 10
          break;
        case "6":
          this.toggle = 25
          break;
        case "w" || "ArrowUp" || "ц":
          this.buttonClick(1)
          break;
        case "a":
          this.buttonClick(2)
          break;
        case "d":
          this.buttonClick(3)
          break;
        case "s":
          this.buttonClick(4)
          break;
        case "ArrowUp":
          this.buttonClick(1)
          break;
        case "ArrowLeft":
          this.buttonClick(2)
          break;
        case "ArrowRight":
          this.buttonClick(3)
          break;
        case "ArrowDown":
          this.buttonClick(4)
          break;
        case "ц":
          this.buttonClick(1)
          break;
        case "ф":
          this.buttonClick(2)
          break;
        case "в":
          this.buttonClick(3)
          break;
        case "ы":
          this.buttonClick(4)
          break;
      }
    }, false);
  }
}
</script>
<style>
.custom-button {
  width: 50px;
  height: 50px;
  padding: 0;
}
</style>
