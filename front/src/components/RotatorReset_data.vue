<template>
  <div>
    <v-btn depressed color="error" @click="dialog=true">
      Delete all data
    </v-btn>
    <v-dialog v-model="dialog" max-width="300">
      <v-card>
        <v-card-title class="text-h5">
          Deleting data
        </v-card-title>

        <v-card-text>
          Are you sure you want to <b>delete all</b> data? This action cannot be undone.
        </v-card-text>

        <v-card-actions>
          <v-spacer></v-spacer>

          <v-btn large x-large color="success" text @click="dialog = false">
            Cancel
          </v-btn>

          <v-btn large x-large text color="error" @click="reset">
            Delete ALL
          </v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>
  </div>
</template>

<script>
import { bus } from '@/event-bus'
export default {
  name: 'RotatorReset_data',
  data() {
    return {
      dialog: false
    }
  },

  methods: {
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
    }

  }
}
</script>
