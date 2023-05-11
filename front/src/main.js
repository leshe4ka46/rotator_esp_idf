import Vue from 'vue'
import './plugins/vuetify'
import App from './App.vue'
import axios from 'axios'
import MdiSvg from '@yeliulee/vue-mdi-svg/v2'
Vue.config.productionTip = false

Vue.prototype.$ajax = axios
axios.defaults.timeout = 500;
Vue.use(MdiSvg)
new Vue({
  render: h => h(App)
}).$mount('#app')
