<template>
  <v-container>
    <v-layout text-xs-center wrap>
      <v-flex xs12 sm6 offset-sm3>
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
          <v-btn v-for="(distance, index) of MoveDistances" :key="index" min-width="40" :value="distance" color="#0288D1"
            :elevation="0" class="white--text ">
            {{ distance }}
          </v-btn>
        </v-btn-toggle>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
/*

*/
import { mdiArrowUpThin, mdiArrowDownThin, mdiArrowLeftThin, mdiArrowRightThin } from '@mdi/js';
export default {
  name: 'RotatorMove',
  data() {
    return {
      MoveDistances: [0.1, 0.25, 1, 5, 10, 90],
      toggle: 1,
      mdiArrowUpThin, mdiArrowDownThin, mdiArrowLeftThin, mdiArrowRightThin
    }
  },
  methods: {
    buttonClick(val) {
      this.$ajax
        .post('/api/v1/data/set/delta', {
          key: localStorage.getItem('rotator_client_id'),
          axis: (val == 1 || val == 4) ? 1 : 0,
          angle: ((val == 2 || val == 4) ? -1 : 1) * this.toggle
        })
    }
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
