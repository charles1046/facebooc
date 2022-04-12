pipeline {
    agent any

    stages {
        stage('Deploy') {
            steps {
                sh './scripts/deploy_updater.sh'
            }
        }
    }
}