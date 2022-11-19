<template>
  <v-container>
    <v-layout text-xs-center wrap>
      <v-flex xs12 sm6 offset-sm3>
        <v-card>

          <v-card-title primary-title>
            <div class="ma-auto">
              <span class="grey--text">IDF version: {{version}}</span>
              <br>
              <span class="grey--text">ESP cores: {{cores}}</span>
              <br>
              <span class="grey--text">ESP revision: {{revision}}</span>
            </div>
          </v-card-title>
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
export default {
  data () {
    return {
      version: null,
      cores: null,
      revision:null
    }
  },
  mounted () {
    this.$ajax
      .get('/api/v1/system/info')
      .then(data => {
        this.version = data.data.version
        this.cores = data.data.cores
        this.revision=data.data.revision
      })
      .catch(error => {
        // eslint-disable-next-line
        console.log(error);
      })
  }
}
</script>
