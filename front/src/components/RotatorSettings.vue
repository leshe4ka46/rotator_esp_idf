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
      <!--<v-tab tab="2" style="font-size: 1em;">
            Удаление данных
          </v-tab>-->
      <!--<v-tab tab="3" style="font-size:1em;">
            Подключение
          </v-tab>-->
    </v-tabs>

    <v-container>
      <v-tabs-items v-model="tab">
        <v-tab-item tab="0">
          <v-card flat >
            <RotatorTools></RotatorTools>
          </v-card>
        </v-tab-item>

        <v-tab-item tab="1">
          <v-card flat>
            <RotatorInfo></RotatorInfo>
          </v-card>
        </v-tab-item>
        <!--<v-tab-item tab="2">
          <v-card flat>
            <RotatorReset_data></RotatorReset_data>
          </v-card>
        </v-tab-item>-->
        <!--<v-tab-item tab="3">
          <v-card flat>
            <RotatorConnection></RotatorConnection>
          </v-card>
        </v-tab-item>-->
      </v-tabs-items>
    </v-container>
  </v-card>
</template>
<script>
/* eslint-disable */
import { bus } from '@/event-bus'

import RotatorConnection from './RotatorConnection.vue'
import RotatorInfo from './RotatorInfo.vue'
import RotatorReset_data from './RotatorReset_data.vue'
import RotatorTools from './RotatorTools.vue'

export default {
  name: 'RotatorSettings',
  components: {
    RotatorConnection, RotatorReset_data, RotatorInfo, RotatorTools
  },
  data() {
    return {
      tab: null,
      is_admin: false,
    };
  },
  created() {
    bus.$on('is_admin', (val) => { this.is_admin = val; });
  }


}


</script>




<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>

</style>