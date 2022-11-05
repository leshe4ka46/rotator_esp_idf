import Vue from 'vue'
import Router from 'vue-router'
import Home from './components/RotatorMain.vue'
import Light from './components/RotatorLight.vue'

import RotatorDataset from './components/RotatorDataset.vue'
Vue.use(Router)

export default new Router({
  mode: 'history',
  base: process.env.BASE_URL,
  routes: [
    {
      path: '/',
      name: 'home',
      component: Home
    },
    {
      path: '/data',
      name: 'data',
      component: RotatorDataset
    },
    {
      path: '/light',
      name: 'light',
      component: Light
    }
  ]
})
