<template>
  <v-container>
    <v-btn depressed color="primary" @click="zeroaxes">
      Сбросить значение граусов Х
    </v-btn>
    <v-sheet color="white" title="DH">
      <v-text-field label="Азимут" v-model="azimut" type="number" @keyup.enter.exact="sendangles" />
      <v-text-field label="Элевация" v-model="elevation" type="number" @keyup.enter.exact="sendangles" />
      <v-btn tile color="success" @click="sendangles">
        Отправить
      </v-btn>
    </v-sheet>
  </v-container>
</template>

<script>
export default {
  name: 'RotatorTools',
  data() {
    return {
      azimut:0,
      elevation:0
    }
  },

  methods: {
    zeroaxes() {
      this.$ajax
        .post('/api/v1/data/set/nullX', {
          key: localStorage.getItem('rotator_client_id')
        })
    },
    sendangles() {
      this.$ajax
        .post('/api/v1/data/set/currentangles', {
          key: localStorage.getItem('rotator_client_id'),
          azimut: Number(this.azimut)*100,
          elevation: Number(this.elevation)*100
        })
    }
  }
}
</script>
