<template>
  <v-container>
    <v-btn depressed color="primary" @click="zeroaxesX">
      Сбросить значение граусов Х
    </v-btn>
    <v-btn depressed color="primary" @click="zeroaxesY">
      Сбросить значение граусов Y
    </v-btn>
    <v-sheet color="white">
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
    zeroaxesX() {
      this.$ajax
        .post('/api/v1/data/set/nullX', {
          key: localStorage.getItem('rotator_client_id')
        })
    },
    zeroaxesY() {
      this.$ajax
        .post('/api/v1/data/set/nullY', {
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
