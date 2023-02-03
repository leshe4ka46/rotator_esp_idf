<template>
  <div>
    <v-btn depressed color="error" @click="reset">
      Delete all data
    </v-btn>
    <v-btn depressed color="error" @click="zero_as5600">
      Сбросить значение всех углов
    </v-btn>
    <v-sheet color="white">
      <v-text-field label="GPS diff" v-model="diffgps" type="number"/>
      <v-btn tile color="success" @click="senddiff">
        Отправить
      </v-btn>
    </v-sheet>
  </div>
</template>

<script>
import { bus } from '@/event-bus'
export default {
  name: 'RotatorDev',
  data() {
    return {
      diffgps:0.1,
    }
  },

  methods: {
    zero_as5600() {
      this.$ajax
        .post('/api/v1/reset/as5600/', {
          key: localStorage.getItem('rotator_client_id')
        })
    },
    reset() {
      this.$ajax
        .post('/api/v1/users/clear', {
          key: localStorage.getItem('rotator_client_id')
        })
        .then(() => {
          // eslint-disable-next-line
          this.dialog = false
          bus.$emit('is_admin', 0)
          bus.$emit('close_dialogs', 0)
        })
        .catch(error => { // eslint-disable-next-line
          console.log(error);
        })
    },
    senddiff() {
      this.$ajax
        .post('/api/v1/data/set/diffgps', {
          key: localStorage.getItem('rotator_client_id'),
          diff: this.diffgps
        })
    },
  }
}
</script>
