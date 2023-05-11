<template>
  <div class="d-flex justify-center">
    <v-card flat max-width="500px">
      <v-card-title primary-title>
        <h4>Вход</h4>
      </v-card-title>
      <div class="container">
        <v-form>
          <v-text-field label="Логин" v-model="login" :error="err_input_login"
            :rules="[() => !!login || 'Тут нужно что-то написать']" required></v-text-field>
          <v-text-field label="Пароль" type="password" v-model="password" :error="err_input_password"
            :rules="[() => !!password || 'Тут нужно что-то написать']" required @keypress.enter.exact="send">
          </v-text-field>
          <v-card-actions>
            <v-btn primary large block @click="send" :loading="login_loading">Вход</v-btn>
          </v-card-actions>
        </v-form>
      </div>
    </v-card>
  </div>

</template>

<script>
import { bus } from '@/event-bus'
export default {
  name: 'RotatorAuth',
  data() {
    return {
      login_loading: false,
      is_admin: false,
      login: '',
      password: '',
      err_input_login:false,
      err_input_password:false
    }
  },
  methods: {
    send() {
      this.login_loading = true
      //console.log(this.login.length);
      if (this.login.length == 0 || this.password.length == 0) {
        if(this.login.length == 0){this.err_input_login=true;}else{this.err_input_login=false;}
        if(this.password.length == 0){this.err_input_password=true;}else{this.err_input_password=false;}

        this.login_loading = false
        return -1;
      }
      this.err_input_login=false;
      this.err_input_password=false;
      this.$ajax
        .post('/api/v1/users/auth', {
          login: this.login,
          password: this.password,
          key: localStorage.getItem('rotator_client_id')
        })
        .then(function (data) {
          if (data.data.role === 'admin') {
            this.is_admin = true;
            bus.$emit('is_admin', 1)
            bus.$emit('close_dialogs')

          }
          this.login_loading = false
        }.bind(this))
        .catch(error => { // eslint-disable-next-line
          console.log(error);
        })
    },
    checkadmin() {
      if (localStorage.getItem('rotator_client_id') != null) {
        this.$ajax
          .post('/api/v1/users/whoami', {
            key: localStorage.getItem('rotator_client_id')
          })
          .then(function (data) {
            if (data.data.role == "admin") {
              this.is_admin = true;
              bus.$emit('is_admin', 1)
              bus.$emit('close_dialogs')

            }
          }.bind(this))
          .catch(error => { // eslint-disable-next-line
            console.log(error);
          })
      }
    }
  },
  created() {
    this.checkadmin()
    bus.$emit('is_admin', this.is_admin ? 1 : 0)
    bus.$on('get_admin', () => {
      bus.$emit('is_admin', this.is_admin ? 1 : 0)
    })
    //setInterval(() => this.checkadmin(),10000)
  }
}
</script>
