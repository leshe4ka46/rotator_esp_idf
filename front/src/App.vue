<template>
  <div id="container">
    <v-app id="inspire">
      <v-navigation-drawer v-model="drawer" fixed app clipped v-if="device == 1">
        <v-list dense v-model="page" rounded>
          <v-list-tile @click="page = 0; drawer = false">
            <v-list-tile-action>
              <MdiSvg>{{ mdiHome }}</MdiSvg>
            </v-list-tile-action>
            <v-list-tile-content>
              <v-list-tile-title>Информация</v-list-tile-title>
            </v-list-tile-content>
          </v-list-tile>

          <v-list-tile @click="page = 1; drawer = false">
            <v-list-tile-action>
              <MdiSvg>{{ mdiChartBoxOutline }}</MdiSvg>
            </v-list-tile-action>
            <v-list-tile-content>
              <v-list-tile-title>Данные</v-list-tile-title>
            </v-list-tile-content>
          </v-list-tile>
        </v-list>
      </v-navigation-drawer>
      <v-toolbar color="#0091EA" dark fixed app clipped-left>
        <v-toolbar-side-icon @click.stop="open_drawer" v-if="device == 1">
          <MdiSvg>{{ mdiMenu }}</MdiSvg>
        </v-toolbar-side-icon>
        <v-toolbar-title @click="page = 0">SPORADIC Rotator</v-toolbar-title>
        <v-spacer></v-spacer>
        <v-btn icon v-if="is_admin == 1" @click="joystick_dialog = true">
          <MdiSvg>{{ mdiController }}</MdiSvg>
        </v-btn>
        <v-btn icon v-if="is_admin == 0" @click.stop="login_dialog = true">
          <MdiSvg>{{ mdiLoginVariant }}</MdiSvg>
        </v-btn>
        <v-btn icon v-if="is_admin == 1" @click="settings_dialog = true">
          <MdiSvg>{{ mdiWrenchCog }}</MdiSvg>
        </v-btn>

      </v-toolbar>
      <v-content>
        <v-container fill-height center style="font-size: 1.2em;" v-if="device == 1">
          <RotatorMain v-if="page == 0" :joy_opened="joystick_dialog"></RotatorMain>
          <keep-alive>
            <RotatorDataset v-if="page == 1 && is_admin == true"></RotatorDataset>
            <RotatorError text="Нужна авторизация" v-if="page == 1 && is_admin == false" />
          </keep-alive>
        </v-container>
        <v-container fill-height style="font-size: 1.2em;" v-if="device == 0">
          <RotatorMain style="max-width: 40%;" :joy_opened="joystick_dialog"></RotatorMain>
          <RotatorDataset v-if="is_admin"></RotatorDataset>
        </v-container>
      </v-content>
    </v-app>
    <v-dialog v-model="settings_dialog" max-width="650px">
      <v-card>
        <RotatorSettings style="font-size: 1.2em;" v-if="settings_dialog"></RotatorSettings>
      </v-card>
    </v-dialog>
    <v-dialog v-model="login_dialog" width="800">
      <v-card>
        <RotatorAuth style="font-size: 1.5em;" v-if="login_dialog"></RotatorAuth>
      </v-card>
    </v-dialog>
    <v-dialog v-model="joystick_dialog" max-width="550px">
      <v-card>
        <RotatorMove :opened="joystick_dialog" style="font-size: 1.2em;" v-if="joystick_dialog"></RotatorMove>
      </v-card>
    </v-dialog>
  </div>
</template>


<script>
import { defineAsyncComponent } from 'vue';
import RotatorLoading from './components/RotatorLoading.vue'
import RotatorError from './components/RotatorError.vue'
const RotatorAuth = defineAsyncComponent({ loader: () => import('./components/RotatorAuth.vue'), loadingComponent: RotatorLoading, errorComponent: RotatorError, timeout: 3000 });
const RotatorSettings = defineAsyncComponent({ loader: () => import('./components/RotatorSettings.vue'), loadingComponent: RotatorLoading, errorComponent: RotatorError, timeout: 3000 });
const RotatorMain = defineAsyncComponent({ loader: () => import('./components/RotatorMain.vue'), loadingComponent: RotatorLoading, errorComponent: RotatorError, timeout: 3000 });
const RotatorDataset = defineAsyncComponent({ loader: () => import('./components/RotatorDataset.vue'), loadingComponent: RotatorLoading, errorComponent: RotatorError, timeout: 3000 });
const RotatorMove = defineAsyncComponent({ loader: () => import('./components/RotatorMove.vue'), loadingComponent: RotatorLoading, errorComponent: RotatorError, timeout: 3000 });
/*import RotatorAuth from './components/RotatorAuth.vue'
import RotatorSettings from './components/RotatorSettings.vue'
import RotatorMain from './components/RotatorMain.vue'
import RotatorDataset from './components/RotatorDataset.vue'
import RotatorMove from './components/RotatorMove.vue';*/
import { bus } from '@/event-bus'
import { mdiMenu, mdiHome, mdiChartBoxOutline, mdiWrenchCog, mdiLoginVariant, mdiLampsOutline, mdiController } from '@mdi/js'
export default {
  name: 'app',
  data() {
    return {
      page: 0,
      mdiWrenchCog,
      mdiLoginVariant,
      mdiMenu,
      mdiHome,
      mdiChartBoxOutline,
      mdiLampsOutline,
      mdiController,
      login_dialog: false,
      drawer: false,
      settings_dialog: false,
      joystick_dialog: false,
      is_admin: false,
      setted_azimut: 0,
      setted_elevation: 0,
      device: 0   // 0 - pc, 1 - mobile
    }
  },
  created() {
    if (localStorage.getItem('rotator_client_id') == null || localStorage.getItem('rotator_client_id').length > 8) {
      try {
        var key = this.makeid(8)
        this.$ajax
          .post('/api/v1/users/add', {
            key: key
          })
          .then(function (data) { // eslint-disable-next-line
            //console.log(data);
            if (data.data.response) {
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
    bus.$on('is_admin', (val) => {
      this.is_admin = val
    })
    bus.$on('close_dialogs', () => {
      this.login_dialog = false
      this.settings_dialog = false
    })
    this.set_device()
    addEventListener("resize", () => { this.set_device() });
    //setInterval(this.set_device, 1000)
  },
  mounted() {

  },
  components: {
    RotatorAuth, RotatorSettings, RotatorDataset, RotatorMain, RotatorMove, RotatorError
  },
  methods: {
    set_device() {
      if (window.innerWidth < 1300) {
        //bus.$emit('device', {'mobile': true})
        this.device = 1
      } else {
        this.device = 0
      }

    },
    open_drawer() {
      if (this.is_admin == true) {
        this.drawer = true;
      }
      else {
        this.login_dialog = true;
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

<style>
body::-webkit-scrollbar {
  width: 0px;
}
.mdi {
  display: flex;
  justify-content: center;
  margin: auto;
  text-align: center;
}
</style>