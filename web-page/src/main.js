import Vue from 'vue'
import './plugins/vuetify'
import App from './App.vue'
import router from './router'
import axios from 'axios'
import '@yeliulee/vue-mdi-svg/src/styles.css'
import MdiSvg from '@yeliulee/vue-mdi-svg/v2'
Vue.config.productionTip = false

Vue.prototype.$ajax = axios
Vue.use(MdiSvg)
new Vue({
  router,
  render: h => h(App)
}).$mount('#app')
