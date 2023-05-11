<template>
  <v-container width="100%">
    <v-layout text-xs-center wrap>
      <v-flex xs12 sm6 offset-sm3>
            <div class="ma-auto">
              <span class="grey--text" @click="dev_mode += 1">Версия IDF: {{ version }}</span>
              <br>
              <span class="grey--text">Ядер ESP32: {{ cores }}</span>
              <br>
              <span class="grey--text">Версия ESP32: {{ revision }}</span>
              <br>
              <span class="grey--text">Версия: 1.0.1</span>
              <br>
              <v-btn tile color="warning" @click="restart_esp">Перезагрузить поворотку
              </v-btn>
            </div>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
import { bus } from '@/event-bus'
export default {
  data() {
    return {
      version: null,
      cores: null,
      revision: null,
      dev_mode: 0
    }
  },
  watch: {
    dev_mode(val) {
      if (val > 10) {
        //alert("Developer mode enabled");
        bus.$emit("dev_mode");
      }
    }
  },
  mounted() {
    this.$ajax
      .get('/api/v1/system/info')
      .then(data => {
        this.version = data.data.version
        this.cores = data.data.cores
        this.revision = data.data.revision
      })
      .catch(error => {
        console.log(error);
      })
  },
  methods: {
    restart_esp() {
      this.$ajax
        .post('/api/v1/system/restart', {
          key: localStorage.getItem('rotator_client_id')
        })
        .catch(error => {
          console.log(error)
        })
    }
  }
}
</script>
