<template>
  <v-container>
    <v-radio-group v-model="val" mandatory>
      <v-radio
        v-for="(n,index) in data"
        :key="index"
        :label="`${n.name}`"
        :value="n.value"
      ></v-radio>
    </v-radio-group>

    <div v-if="val=='1'">Для подключения надо установить приложение ESP SoftAP Provisioning
      <br>
      <p class="text-wrap">
        <a class="overflow-auto" href="https://play.google.com/store/apps/details?id=com.espressif.provsoftap">https://play.google.com/store/apps/details?id=com.espressif.provsoftap</a>
      </p>
    </div>
    <v-btn tile color="success" @click="save_wireless_mode">Сохранить</v-btn>
  </v-container>

</template>

<script>
export default {
  name: 'RotatorConnection',
  data () {
    return {
      data:[{name:"AP",value:"0"},{name:"WIFI",value:"1"}],
      val:"0",
    }
  },
  created () {},
  methods: {
    save_wireless_mode(){
      this.$ajax
        .post('/api/v1/data/set/wifi_mode', {
          key: localStorage.getItem('rotator_client_id'),
          mode: Number(this.val)
        })
        .catch(error => {
          console.log(error)
        })
    }
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
