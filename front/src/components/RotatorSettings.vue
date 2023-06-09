<template v-slot:extension>
  <v-card>

    <v-tabs grow v-model="tab">


      <v-tabs-slider color="blue"></v-tabs-slider>
      <v-tab tab="0" style="font-size:1em;">
        Настройки
      </v-tab>
      <v-tab tab="1" style="font-size:1em;">
        Информация
      </v-tab>
      <v-tab tab="2" style="font-size: 1em;" v-if="dev_mode == true">
        Dev mode
      </v-tab>
    </v-tabs>

    <v-container>
      <v-tabs-items v-model="tab">
        <v-tab-item tab="0">
          <v-card flat>
            <RotatorTools v-if="loaded.includes(0) || tab == 0"></RotatorTools>
          </v-card>
        </v-tab-item>

        <v-tab-item tab="1">
          <v-card flat>
            <RotatorInfo v-if="loaded.includes(1) || tab == 1"></RotatorInfo>
          </v-card>
        </v-tab-item>
        <v-tab-item tab="2">
          <v-card flat>
            <RotatorDev v-if="loaded.includes(2) || tab == 2"></RotatorDev>
          </v-card>
        </v-tab-item>
      </v-tabs-items>
    </v-container>
  </v-card>
</template>

<script>
import { defineAsyncComponent } from 'vue';
import { bus } from '@/event-bus'
import RotatorLoading from './RotatorLoading.vue'
import RotatorError from './RotatorError.vue'
const RotatorInfo = defineAsyncComponent({ loader: () => import('./RotatorInfo.vue'), loadingComponent: RotatorLoading, errorComponent: RotatorError, timeout: 3000 });
const RotatorDev = defineAsyncComponent({ loader: () => import('./RotatorDev.vue'), loadingComponent: RotatorLoading, errorComponent: RotatorError, timeout: 3000 });
const RotatorTools = defineAsyncComponent({ loader: () => import('./RotatorTools.vue'), loadingComponent: RotatorLoading, errorComponent: RotatorError, timeout: 3000 });
/*import RotatorInfo from './RotatorInfo.vue'
import RotatorDev from './RotatorDev.vue'
import RotatorTools from './RotatorTools.vue'*/

export default {
  name: 'RotatorSettings',
  components: {
    RotatorDev, RotatorInfo, RotatorTools
  },
  data() {
    return {
      tab: null,
      is_admin: false,
      dev_mode: false,
      loaded: [],
    };
  },
  watch: {
    tab(val) {
      if (!this.loaded.includes(val)) {
        this.loaded.push(val);
      }
    }
  },
  created() {
    bus.$on('is_admin', (val) => { this.is_admin = val; });
    bus.$on("dev_mode", () => { this.dev_mode = true; });
  }
}
</script>