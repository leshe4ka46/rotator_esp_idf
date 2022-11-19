<template>
  <div>
    <v-app id="inspire">
      <v-navigation-drawer v-model="drawer" fixed app clipped>
        <v-list dense v-model="list_item" rounded>
          <v-list-tile to="/">
            <v-list-tile-action>
              <MdiSvg>{{ mdiHome }}</MdiSvg>
            </v-list-tile-action>
            <v-list-tile-content>
              <v-list-tile-title>Home</v-list-tile-title>
            </v-list-tile-content>
          </v-list-tile>

          <v-list-tile to="/data" v-if="is_admin == true">
            <v-list-tile-action>
              <MdiSvg>{{ mdiChartBoxOutline }}</MdiSvg>
            </v-list-tile-action>
            <v-list-tile-content>
              <v-list-tile-title>Data</v-list-tile-title>
            </v-list-tile-content>
          </v-list-tile>
          <!--<v-list-tile to="/light" v-if="is_admin==true">
            <v-list-tile-action>
              <MdiSvg>{{mdiLampsOutline}}</MdiSvg>
            </v-list-tile-action>
            <v-list-tile-content>
              <v-list-tile-title>Light</v-list-tile-title>
            </v-list-tile-content>
          </v-list-tile>-->

        </v-list>
      </v-navigation-drawer>
      <v-toolbar color="red accent-4" dark fixed app clipped-left>
        <v-toolbar-side-icon @click.stop="open_drawer">
          <MdiSvg>{{ mdiMenu }}</MdiSvg>
        </v-toolbar-side-icon>
        <v-toolbar-title>ESP Rotator</v-toolbar-title>
        <v-spacer></v-spacer>
        <v-btn icon v-if="is_admin == 0">
          <div @click.stop="login_dialog = true">
            <MdiSvg>{{ mdiLoginVariant }}</MdiSvg>
          </div>
        </v-btn>
        <v-btn icon v-if="is_admin == 1">
          <div @click="settings_dialog = true">
            <MdiSvg>{{ mdiWrenchCog }}</MdiSvg>
          </div>
        </v-btn>

      </v-toolbar>

      <v-dialog v-model="settings_dialog" max-width="85%">
        <v-card>
          <v-card-title style="font-size: 1.2em;">
            Настройки
          </v-card-title>

          <RotatorSettings style="font-size: 1.2em;"></RotatorSettings>

        </v-card>
      </v-dialog>
      <v-dialog v-model="login_dialog" width="800">
        <v-card>
          <RotatorAuth style="font-size: 1.5em;"></RotatorAuth>
        </v-card>
      </v-dialog>
      <v-content>
        <v-container center fill-height>
          <router-view :key="$route.path" style="font-size: 1.2em;"></router-view>
        </v-container>
      </v-content>
    </v-app>
  </div>
</template>


<script>
import RotatorAuth from './components/RotatorAuth.vue'
import RotatorSettings from './components/RotatorSettings.vue'
import { bus } from '@/event-bus'
import { mdiMenu, mdiHome, mdiChartBoxOutline, mdiWrenchCog, mdiLoginVariant, mdiLampsOutline } from '@mdi/js'
export default {
  name: 'app',
  data() {
    return {
      list_item: 0,
      mdiWrenchCog,
      mdiLoginVariant,
      mdiMenu,
      mdiHome,
      mdiChartBoxOutline,
      mdiLampsOutline,
      dialog: false,
      login_dialog: false,
      drawer: false,
      settings_dialog: false,
      mode: true,
      isLoading: true,
      is_admin: false,
      setted_azimut: 0,
      setted_elevation: 0
    }
  },
  created() {
    if (localStorage.getItem('rotator_client_id') == null || localStorage.getItem('rotator_client_id').length > 8) {
      try {
        var key = this.makeid(8)
        this.$ajax
          .post('/api/v1/users/add', {
            key: this.makeid(8)
          })
          .then(function (data) { // eslint-disable-next-line
            console.log(data);
            if (data.data === 'OK') {
              localStorage.setItem('rotator_client_id', key)
            }
          })
          .catch(error => { // eslint-disable-next-line
            console.log(error);
          })

      } catch (e) {
        // eslint-disable-next-line
        console.log(e)
      }
    }

    bus.$on('save_data_angles', (data) => {
      this.setted_azimut = data.azimut
      this.setted_elevation = data.elevation
    })
    bus.$on('get_data_angles', () => {
      bus.$emit('return_data_angles', { azimut: Number(this.setted_azimut), elevation: Number(this.setted_elevation) })
    })
    bus.$on('is_admin', (val) => {
      this.is_admin = val
    })
    bus.$on('close_dialogs', () => {
      this.login_dialog = false
      this.settings_dialog = false
    })
  },
  mounted() {
    setTimeout(() => {
      this.isLoading = false
    }, 100)
  },
  components: {
    RotatorAuth, RotatorSettings/* RotatorSettings, RotatorMain */
  },
  methods: {
    open_drawer() {
      if (this.is_admin == true) {
        this.drawer = !this.drawer
      }
      else{
        this.login_dialog=true;
      }
    },
    makeid(length) {
      var result = ''
      var characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
      var charactersLength = characters.length
      for (var i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() * charactersLength))
      } return result
    }

  }

}
</script>
